#pragma once
#include <cstdint>
#include <array>
#include "menu_struct.hpp"

using namespace protocol;

namespace menu
{

    /// “Is this index a real Page?”
    static inline bool isPageItem(uint8_t itemIndex)
    {
        return itemIndex < PAGE_COUNT;
    }

    /// Map a page‐list index into your Page enum (valid only if isPageItem() is true)
    static inline Page itemToPage(uint8_t itemIndex)
    {
        return static_cast<Page>(itemIndex);
    }

    /// “Is this index one of your popup workflows?”
    static inline bool isWorkflowItem(uint8_t itemIndex)
    {
        return itemIndex >= PAGE_COUNT && itemIndex < MENU_ITEM_COUNT;
    }

    /// Map a workflow‐list index into your PopupWorkflow (valid only if isWorkflowItem() is true)
    static inline const PopupWorkflow &itemToWorkflow(uint8_t itemIndex)
    {
        return popupWorkflows[itemIndex - PAGE_COUNT];
    }
  struct PopupState
    {
        Workflow workflowIndex = Workflow::Count; ///< invalid until set
        uint8_t stepIndex = 0;                    ///< 0..stepCount-1
        uint8_t slotIndex = 0;                    ///< voice/project slot cursor
        char editName[5] = {'\0'};                ///< 4-char rename buffer
        std::vector<NameEntry> listItems;         ///< current list of names
    };
    inline PopupMode getCurrentPopupMode(const PopupState &p)
    {
        size_t wf = static_cast<size_t>(p.workflowIndex);
        if (wf >= WORKFLOW_COUNT)
            return PopupMode::Count;
        const auto &workflow = popupWorkflows[wf];
        uint8_t step = p.stepIndex;
        if (step >= workflow.stepCount)
            return PopupMode::Count;
        return workflow.steps[step].mode;
    }
  

    static constexpr char nameAlphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    static constexpr std::size_t nameAlphabetSize = sizeof(nameAlphabet) - 1;

    // 8) Helpers to advance/retreat
    inline bool advancePopup(PopupState &p)
    {
        auto w = static_cast<size_t>(p.workflowIndex);
        if (w >= static_cast<size_t>(Workflow::Count))
            return false;
        auto &wf = popupWorkflows[w];
        if (p.stepIndex + 1 < wf.stepCount)
        {
            ++p.stepIndex;
            return true;
        }
        return false;
    }

    inline bool retreatPopup(PopupState &p)
    {
        if (p.stepIndex > 0)
        {
            --p.stepIndex;
            return true;
        }
        return false;
    }

    struct MenuState
    {
        AppMode mode;                   // which of the 3 modes we’re in
        uint8_t menuItemIndex;          ///< Current menu page
        uint8_t voice;                  ///< Current voice number (1..N)
        uint8_t channel;                ///< Current channel for that voice
        uint8_t volume;                 ///< Current volume for that voice
        int8_t fieldValues[MAX_FIELDS]; ///< Current values for each knob
        bool shouldAutoSave;

        PopupState popup; ///< active load/save overlay
        std::array<EncoderRange, MAX_FIELDS> encoderRanges;
    };
}
