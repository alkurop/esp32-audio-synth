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

void Menu::changeValuePopup(uint8_t knob, int8_t pos)
{
    auto &ps = state.popup;
    // grab the current mode via helper
    auto mode = getCurrentPopupMode(ps);

    if (isListPopup(mode))
    {
        // only encoder 0 matters: selects slotIndex in the list
        if (knob == 0)
        {
            int maxIdx = ps.listItems.empty() ? 0 : int(ps.listItems.size()) - 1;
            ps.slotIndex = std::clamp<int>(pos, 0, maxIdx);
        }
    }
    else if (isInputPopup(mode))
    {
        // each encoder i (0..3) sets the character at editName[i]
        if (knob < MaxFieldsPerPage)
        {
            int idx = std::clamp<int>(pos, 0, nameAlphabetSize - 1);
            ps.editName[knob] = nameAlphabet[idx];
            // move the visible “cursor” to this position
            ps.slotIndex = knob;
        }
    }
    else
    {
        // confirmation popups don’t react to knobs
        return;
    }

    // update ranges in case they depend on slotIndex or list size
    state.encoderRanges = calcEncoderRanges();
    notify();
}

void Menu::enterPopup()
{
    // 1) flip into Popup mode
    state.mode = AppMode::Popup;

    // 2) pick workflow from menuItemIndex
    uint8_t rawWf = state.menuItemIndex - PageCount;
    rawWf = std::min<uint8_t>(rawWf, static_cast<uint8_t>(Workflow::Count) - 1);

    // 3) reset PopupState
    state.popup = {};
    state.popup.workflowIndex = static_cast<Workflow>(rawWf);
    state.popup.stepIndex = 0;
    state.popup.slotIndex = 0;

    // 4) if the first step is a list, fetch names
    auto firstMode = getCurrentPopupMode(state.popup);
    if (isListPopup(firstMode))
    {
        if (firstMode == PopupMode::LoadVoiceList ||
            firstMode == PopupMode::SaveVoiceList)
        {
            state.popup.listItems = paramStore.listVoiceNames();
        }
        else
        {
            state.popup.listItems = paramStore.listProjectNames();
        }
    }

    // 5) recompute ranges & notify
    state.encoderRanges = calcEncoderRanges();
    notify();
}

void Menu::closePopup()
{
    // go back to list mode
    state.mode = AppMode::MenuList;

    // clear popup state
    state.popup = {};

    state.encoderRanges = calcEncoderRanges();
    notify();
}

bool Menu::updatePopupStateForward()
{
    // 1) Advance to the next step
    bool didAdvance = advancePopup(state.popup);
    if (!didAdvance)
        return false;

    // 2) What mode did we land on?
    auto mode = getCurrentPopupMode(state.popup);

    // 3) Fire off any “confirm” events
    switch (mode)
    {
    case PopupMode::LoadVoiceConfirm:
    {
        eventCb(LoadVoiceEvent{state.popup.slotIndex});
        break;
    }

    case PopupMode::LoadProjectConfirm:
    {
        eventCb(LoadProjectEvent{state.popup.slotIndex});
        break;
    }

    case PopupMode::SaveVoiceConfirm:
    {
        // grab the 4-char name as std::string
        std::string name(state.popup.editName, state.popup.editName + 4);
        eventCb(SaveVoiceEvent{
            state.popup.slotIndex,
            name});
        break;
    }

    case PopupMode::SaveProjectConfirm:
    {
        std::string name(state.popup.editName, state.popup.editName + 4);
        eventCb(SaveProjectEvent{
            state.popup.slotIndex,
            name});
        break;
    }

    default:
        // no event on other steps
        break;
    }

    return true;
}

bool Menu::updatePopupStateBack()
{
    // retreat one step
    bool ok = retreatPopup(state.popup);
    if (ok)
    {
        auto mode = getCurrentPopupMode(state.popup);
        if (isListPopup(mode))
        {
            // repopulate listItems on re-entering a list step
            if (mode == PopupMode::LoadVoiceList ||
                mode == PopupMode::SaveVoiceList)
            {
                state.popup.listItems = paramStore.listVoiceNames();
            }
            else
            {
                state.popup.listItems = paramStore.listProjectNames();
            }
            state.popup.slotIndex = 0;
        }
    }
    return ok;
}
