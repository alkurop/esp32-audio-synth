// Menu.cpp
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <algorithm>
#include <cstdint>
#include <utility>
#include <esp_log.h>
#include <cstring>

#include "menu.hpp"
#include "menu_struct.hpp"

#define TAG "Menu"

using namespace menu;

Menu::Menu(uint8_t voiceCount)
    : paramStore(), state{}, voiceCount(voiceCount), cache(voiceCount) // value‐initialize everything
{
    state.mode = AppMode::Loading;
}

void Menu::init(DisplayCallback displayCb, FieldUpdateCallback updateCb)
{
    displayCallback = std::move(displayCb);
    cache.setCallback(updateCb);
    notify();
    // prime the encoder ranges & draw initial screen
    state.encoderRanges = calcEncoderRanges();
    initAutosaveTask();
    // wait for synth device to init, because initing menu will cause sending
    // the synth settings over i2c
    vTaskDelay(pdMS_TO_TICKS(1000));
    loadProject(-1);
}

void Menu::updateAfterAutoLoad()
{
    state.mode = AppMode::MenuList;
    state.encoderRanges = calcEncoderRanges();
    notify();
}

void Menu::enterMenuPage()
{
    if (state.mode == AppMode::Popup)
    {
        // advance or fall out
        if (!updatePopupStateForward())
        {
            state.mode = AppMode::MenuList;
        }
    }
    else
    {
        // select Page vs Popup
        if (isPageItem(state.menuItemIndex))
        {
            state.mode = AppMode::Page;
            updatePageFromCache();
        }
        else
        {
            state.mode = AppMode::Popup;
            enterPopup(); // initializes state.popup
        }
    }

    state.encoderRanges = calcEncoderRanges();
    notify();
}

void Menu::exitPage()
{
    if (state.mode == AppMode::Popup)
    {
        if (!updatePopupStateBack())
        {
            closePopup();
        }
    }
    else
    {
        state.mode = AppMode::MenuList;
    }

    state.encoderRanges = calcEncoderRanges();
    notify();
}

void Menu::rotateKnob(uint8_t knob, int16_t pos)
{
    switch (state.mode)
    {
    case AppMode::MenuList:
        changeValueMenuList(knob, pos);
        break;
    case AppMode::Page:
        changeValuePage(knob, pos);
        state.shouldAutoSave = true;
        break;
    case AppMode::Popup:
        changeValuePopup(knob, pos);
        break;
    default:
        break;
    }
}

void Menu::changeValueMenuList(uint8_t knob, int16_t pos)
{
    switch (knob)
    {
    case MenuPosition::List:
        state.menuItemIndex = pos;
        break;
    case MenuPosition::None:
        return;
    case MenuPosition::Channel:
    {
        FieldUpdateList updates = {
            FieldUpdate{state.voiceIndex, static_cast<uint8_t>(Page::VolChan), static_cast<uint8_t>(ChannelField::Chan), pos}};
        cache.set(updates);
        state.shouldAutoSave = true;
        break;
    }
    case MenuPosition::Volume:
    {
        FieldUpdateList updates = {
            FieldUpdate{state.voiceIndex, static_cast<uint8_t>(Page::VolChan), static_cast<uint8_t>(ChannelField::Vol), pos}};
        cache.set(updates);
        state.shouldAutoSave = true;
        break;
    }
    default:
        return;
    }

    // refresh dependent values
    state.channel = cache.get(state.voiceIndex, Page::VolChan, 0);
    state.volume = cache.get(state.voiceIndex, Page::VolChan, 1);
    state.encoderRanges = calcEncoderRanges();
    notify();
}

void Menu::changeValuePage(uint8_t knob, int16_t pos)
{
    const auto &pi = menuPages[state.menuItemIndex];
    if (knob >= pi.fieldCount)
        return;

    const auto &fi = pi.fields[knob];
    int16_t newVal;
    if (fi.type == FieldType::Range)
    {
        newVal = std::clamp<int16_t>(pos, fi.min, fi.max);
    }
    else
    {
        newVal = pos % fi.optCount;
    }

    FieldUpdateList updates = {
        FieldUpdate{
            .voiceIndex = state.voiceIndex,
            .pageByte = static_cast<uint8_t>(itemToPage(state.menuItemIndex)),
            .field = knob,
            .value = newVal}};
    cache.set(updates);
    // update snapshot
    state.fieldValues[knob] = cache.get(
        state.voiceIndex,
        itemToPage(state.menuItemIndex),
        knob);
    state.encoderRanges = calcEncoderRanges();
    notify();
}

std::array<EncoderRange, 4> Menu::calcEncoderRanges()
{
    switch (state.mode)
    {
    case AppMode::MenuList:
        return getEncoderRangesMenuList(state);
    case AppMode::Page:
        return getEncoderRangesPage(itemToPage(state.menuItemIndex), state);
    case AppMode::Popup:
        return getEncoderRangesPopup(state.popup);
    default:
        return {};
    }
}

void Menu::notify()
{
    if (!displayCallback)
        return;
    displayCallback(state);
}

void Menu::updatePageFromCache()
{
    // Figure out which page we’re on
    auto page = itemToPage(state.menuItemIndex);

    // How many knobs/fields does that page have?
    uint8_t count = menuPages[size_t(page)].fieldCount;

    // Pull each one from the cache…
    for (uint8_t knob = 0; knob < count; ++knob)
    {
        state.fieldValues[knob] = cache.get(state.voiceIndex, page, knob);
    }
}

void Menu::voiceUp()
{
    if (state.mode == AppMode::Popup)
        return;

    if (state.voiceIndex > 0)
    {
        state.voiceIndex--;
        updateVoiceUp();
    }
}
void Menu::voiceDown()
{
    if (state.mode == AppMode::Popup)
        return;
    if (state.voiceIndex < voiceCount - 1)
    {
        state.voiceIndex++;
        updateVoiceUp();
    }
}

void Menu::updateVoiceUp()
{
    // refresh dependent values
    state.channel = cache.get(state.voiceIndex, Page::VolChan, 0);
    state.volume = cache.get(state.voiceIndex, Page::VolChan, 1);
    state.encoderRanges = calcEncoderRanges();

    if (state.mode == AppMode::Page)
        updatePageFromCache();
    notify();
}
