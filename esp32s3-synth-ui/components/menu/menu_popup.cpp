
#include <algorithm>
#include <cstdint>
#include <utility>
#include <esp_log.h>
#include <cstring>

#include "menu.hpp"
#include "menu_struct.hpp"
static const char *TAG = "Menu";
using namespace menu;

void Menu::changeValuePopup(uint8_t knob, int8_t pos) {}

void Menu::enterPopup()
{
    using namespace menu;

    // 1) Switch mode
    mode = AppMode::Popup;

    // 2) Pick which workflow from your linear itemIndex
    //    (items 0..PageCount-1 are pages, the rest are workflows)
    uint8_t rawWf = itemIndex - PageCount;
    rawWf = std::min<uint8_t>(rawWf, static_cast<uint8_t>(Workflow::Count) - 1);

    // 3) Reset your PopupState member
    popup = {}; // zero‐initialize everything, including listItems
    popup.workflowIndex = static_cast<Workflow>(rawWf);
    popup.stepIndex = 0; // first step
    popup.slotIndex = 0; // cursor at top

    // 4) If that first step wants a list, grab it now
    const auto &firstEntry = popupWorkflows[rawWf].steps[0];
    if (isListPopup(firstEntry.mode))
    {
        switch (firstEntry.mode)
        {
        case PopupMode::LoadVoiceList:
        case PopupMode::SaveVoiceList:
            popup.listItems = paramStore.listVoiceNames();
            break;

        case PopupMode::LoadProjectList:
        case PopupMode::SaveProjectList:
            popup.listItems = paramStore.listProjectNames();
            break;

        default:
            popup.listItems.clear();
            break;
        }
    }

    // 5) Finally, tell the display
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
