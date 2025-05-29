// Menu.cpp

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
    : voiceCount(voiceCount), cache(voiceCount), paramStore(), state{} // value‐initialize everything
{
    // Set sensible defaults in state:
    state.mode = AppMode::MenuList;
    state.menuItemIndex = 0;
    state.voice = 0;
    // channel & volume come from cache:
    state.channel = cache.get(0, Page::Channel, 0);
    state.volume = cache.get(0, Page::Channel, 1);
    // fieldValues already zero
    // popup already default‐constructed (invalid workflowIndex)
}

void Menu::init(DisplayCallback displayCb)
{
    displayCallback = std::move(displayCb);
    // prime the encoder ranges & draw initial screen
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

void Menu::exitMenuPage()
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

void Menu::rotateKnob(uint8_t knob, uint8_t pos)
{
    switch (state.mode)
    {
    case AppMode::MenuList:
        changeValueMenuList(knob, pos);
        break;
    case AppMode::Page:
        changeValuePage(knob, pos);
        break;
    case AppMode::Popup:
        changeValuePopup(knob, pos);
        break;
    }
}

void Menu::changeValueMenuList(uint8_t knob, uint8_t pos)
{
    switch (knob)
    {
    case 0:
        state.menuItemIndex = pos;
        break;
    case 1:
        state.voice = pos;
        break;
    case 2:
        cache.set(state.voice, Page::Channel, static_cast<uint8_t>(ChannelField::Chan), pos);
        break;
    case 3:
        cache.set(state.voice, Page::Channel,
                  static_cast<uint8_t>(ChannelField::Vol), pos);
        break;
    default:
        return;
    }

    // refresh dependent values
    state.channel = cache.get(state.voice, Page::Channel, 0);
    state.volume = cache.get(state.voice, Page::Channel, 1);
    state.encoderRanges = calcEncoderRanges();
    notify();
}

void Menu::changeValuePage(uint8_t knob, uint8_t pos)
{
    const auto &pi = menuPages[state.menuItemIndex];
    if (knob >= pi.fieldCount)
        return;

    const auto &fi = pi.fields[knob];
    int newVal;
    if (fi.type == FieldType::Range)
    {
        newVal = std::clamp<int>(pos, fi.min, fi.max);
    }
    else
    {
        newVal = pos % fi.optCount;
    }

    cache.set(state.voice, itemToPage(state.menuItemIndex), knob, newVal);

    // update snapshot
    state.fieldValues[knob] = cache.get(
        state.voice,
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
        state.fieldValues[knob] = cache.get(state.voice, page, knob);
    }
}
