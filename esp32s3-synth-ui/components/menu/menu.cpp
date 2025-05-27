
#include <algorithm>
#include <cstdint>
#include <utility>
#include <esp_log.h>
#include <cstring>

#include "menu.hpp"
#include "menu_struct.hpp"
static const char *TAG = "Menu";

namespace menu
{

    Menu::Menu(uint8_t voiceCount) : voiceCount(voiceCount), cache(voiceCount), popup(), paramStore() {}

    void Menu::init(DisplayCallback displayCallback)
    {
        displayCb = std::move(displayCallback);
        // initial draw
        notify();
    }

    void Menu::enterMenuPage()
    {
        if (mode == AppMode::Popup)
        {
            updatePopupStateForward(); // ignore result
        }
        else
        {
            mode = isPageItem(itemIndex) ? AppMode::Page : AppMode::Popup;
            if (mode == AppMode::Popup)
            {
                enterPopup();
            }
        }
        notify();
    }

    void Menu::exitMenuPage()
    {
        if (mode == AppMode::Popup)
        {
            bool didGoBack = updatePopupStateBack();
            if (!didGoBack)
            {
                closePopup();
            }
        }
        else
        {
            mode = AppMode::MenuList;
        }
        // todo navigate backward in popup workflow
        notify();
    }
    bool Menu::updatePopupStateForward()
    {
        // Simply try to advance the stepIndex in popup.
        // Returns true if we moved forward, false if already at last step.
        return menu::advancePopup(popup);
    }

    bool Menu::updatePopupStateBack()
    {
        // Simply try to retreat the stepIndex in popup.
        // Returns true if we moved back, false if already at first step.
        return menu::retreatPopup(popup);
    }

    void Menu::enterPopup()
    {
        // 1) Switch into popup mode
        mode = AppMode::Popup;

        // 2) Figure out which workflow from the menu index
        uint8_t rawWf = itemIndex - menu::pageCnt;
        // clamp to [0 .. Workflow::Count-1]
        rawWf = std::min<uint8_t>(rawWf, static_cast<uint8_t>(menu::Workflow::Count) - 1);

        // 3) Initialize the new popup state
        auto &p = popup; // alias
        p = {};          // clears workflowIndex to Count, stepIndex to 0, slotIndex to -1, editName to ""
        p.workflowIndex = static_cast<menu::Workflow>(rawWf);
        p.stepIndex = 0;       // first step
        p.slotIndex = 0;       // reset slot cursor (or whatever default you want)
        p.nameEditing = false; // not yet in the rename sub-step
        // editName is already all '\0' thanks to p = {};

        notify();
    }

    void Menu::closePopup()
    {
        // 1) Back to the menu list
        mode = AppMode::MenuList;

        // 2) Clear out popup state entirely
        auto &p = popup;
        p = {}; // workflowIndex = Count (invalid), stepIndex = 0, slotIndex = -1, editName = ""
                // nameEditing = false

        notify();
    }
    
    void Menu::rotateKnob(uint8_t knob, int8_t pos)
    {
        switch (mode)
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

    void Menu::changeValuePopup(uint8_t knob, int8_t pos) {}

    void Menu::changeValuePage(uint8_t knob, int8_t pos)
    {
        // ─── edit mode ────────────────────────────────────────
        // knobs 0..3 map to the page’s fields 0..3, pos is absolute
        const auto &pi = menuPages[itemIndex];
        if (knob > 3 || knob >= pi.fieldCount)
            return;
        const auto &fi = pi.fields[knob];

        int newVal;
        if (fi.type == FieldType::Range)
        {
            newVal = std::clamp<int>(pos, fi.min, fi.max);
        }
        else
        {
            // for options, pos wraps around
            newVal = int(pos) % fi.optCount;
        }
        cache.set(voice, itemToPage(itemIndex), knob, newVal);
        notify();
    }
    void Menu::changeValueMenuList(uint8_t knob, int8_t pos)
    {
        // ─── navigation mode ───────────────────────────────────
        switch (knob)
        {
        case 0:
        { // page select: pos is absolute index
            itemIndex = pos;
            break;
        }
        case 1:
        { // voice select: pos is absolute voice (0..voiceCount-1)
            voice = pos;
            break;
        }
        case 2:
        { // channel (absolute)
            cache.set(voice, Page::Channel, static_cast<uint8_t>(ChannelField::Chan), pos);
            break;
        }
        case 3:
        { // volume (absolute)
            cache.set(voice, Page::Channel, static_cast<uint8_t>(ChannelField::Vol), pos);
            break;
        }
        default:
            return;
        }
        notify();
    }

    std::array<EncoderRange, 4> Menu::calcEncoderRanges()
    {
        switch (mode)
        {
        case AppMode::MenuList:
            return getEncoderRangesMenuList(voiceCount);
        case AppMode::Page:
            return getEncoderRangesPage(itemToPage(itemIndex));
        case AppMode::Popup:
            return getEncoderRangesPopup(popup, &paramStore);
        default:
            return {};
        }
    }
    void Menu::notify()
    {
        if (!displayCb)
            return;
        MenuState s;
        s.mode = mode;
        s.popup = popup;
        s.voice = voice;
        s.menuItemIndex = itemIndex;
        s.channel = cache.get(voice, Page::Channel, 0);
        s.volume = cache.get(voice, Page::Channel, 1);
        if (isPageItem(itemIndex))
        {
            for (int i = 0; i < 4; ++i)
                s.fieldValues[i] = cache.get(voice, itemToPage(itemIndex), i);
        }
        s.popup = popup;
        s.encoderRanges = calcEncoderRanges();
        displayCb(s);
    }

} // namespace menu
