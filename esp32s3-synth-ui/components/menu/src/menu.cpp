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

static const char *TAG = "Menu";

using namespace menu;

Menu::Menu(uint8_t voiceCount)
    : paramStore(), state{}, voiceCount(voiceCount), cache(voiceCount) // value‐initialize everything
{
    state.mode = AppMode::MenuList;
}

void Menu::init(DisplayCallback displayCb, UpdateCallback updateCb)
{
    displayCallback = std::move(displayCb);
    cache.setCallback(updateCb);
    // prime the encoder ranges & draw initial screen
    state.encoderRanges = calcEncoderRanges();
    initAutosaveTask();
      // wait for synth device to init, because initing menu will cause sending
     // the synth settings over i2c
    vTaskDelay(pdMS_TO_TICKS(1000));
    loadProject(-1);
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
    }
}

void Menu::changeValueMenuList(uint8_t knob, int16_t pos)
{
    switch (knob)
    {
    case 0:
        state.menuItemIndex = pos;
        break;
    case 1:
        state.voiceIndex = pos;
        break;
    case 2:
    {
        FieldUpdateList updates = {
            FieldUpdate{state.voiceIndex, static_cast<uint8_t>(Page::Channel), static_cast<uint8_t>(ChannelField::Chan), pos}};
        cache.set(updates);
        state.shouldAutoSave = true;
        break;
    }
    case 3:
    {
        FieldUpdateList updates = {
            FieldUpdate{state.voiceIndex, static_cast<uint8_t>(Page::Channel), static_cast<uint8_t>(ChannelField::Vol), pos}};
        cache.set(updates);
        state.shouldAutoSave = true;
        break;
    }
    default:
        return;
    }

    // refresh dependent values
    state.channel = cache.get(state.voiceIndex, Page::Channel, 0);
    state.volume = cache.get(state.voiceIndex, Page::Channel, 1);
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
        return getEncoderRangesMenuList(voiceCount, state);
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
