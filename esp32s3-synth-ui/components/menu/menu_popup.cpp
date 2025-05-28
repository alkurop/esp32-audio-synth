
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

