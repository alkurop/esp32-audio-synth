#pragma once
#include <array>
#include "menu_struct.hpp"
#include "param_store.hpp"

namespace menu
{

    static inline std::array<EncoderRange, 4> getEncoderRangesMenuList(
        uint8_t voiceCount)
    {
        std::array<EncoderRange, 4> R;

        // page selector
        R[0] = {0, static_cast<uint8_t>(menuItemCnt - 1)};
        // voice selector
        R[1] = {0, static_cast<uint8_t>(voiceCount - 1)};

        // channel on Channel page
        const auto &ci = menuPages[size_t(Page::Channel)];
        {
            const auto &fi = ci.fields[size_t(ChannelField::Chan)];
            if (fi.type == FieldType::Range)
                R[2] = EncoderRange{static_cast<uint8_t>(fi.min), static_cast<uint8_t>(fi.max)};
            else
                R[2] = EncoderRange{0, static_cast<uint8_t>(fi.optCount - 1)};
        }
        // volume on Channel page
        {
            const auto &fi = ci.fields[size_t(ChannelField::Vol)];
            if (fi.type == FieldType::Range)
                R[3] = EncoderRange{static_cast<uint8_t>(fi.min), static_cast<uint8_t>(fi.max)};
            else
                R[3] = EncoderRange{0, static_cast<uint8_t>(fi.optCount - 1)};
        }

        return R;
    }

    /// Edit‐page ranges: knob0–3 map directly to the current page’s fields
    static inline std::array<EncoderRange, 4> getEncoderRangesPage(
        const Page &page)
    {
        std::array<EncoderRange, 4> R;
        const auto &pi = menuPages[size_t(page)];

        for (int k = 0; k < 4; ++k)
        {
            if (k < pi.fieldCount)
            {
                const auto &fi = pi.fields[k];
                if (fi.type == FieldType::Range)
                    R[k] = EncoderRange{static_cast<uint8_t>(fi.min), static_cast<uint8_t>(fi.max)};
                else
                    R[k] = EncoderRange{0, static_cast<uint8_t>(fi.optCount - 1)};
            }
            else
            {
                R[k] = EncoderRange{0, 0};
            }
        }
        return R;
    }

    static inline std::array<EncoderRange, 4> getEncoderRangesPopup(
        const PopupState &popupState)
    {
        using namespace menu;
        std::array<EncoderRange, 4> R{}; // all {0,0}

        // 1) Validate workflow + step
        size_t wfIdx = static_cast<size_t>(popupState.workflowIndex);
        if (wfIdx >= workflowCnt)
            return R;
        const auto &wf = popupWorkflows[wfIdx];

        uint8_t step = popupState.stepIndex;
        if (step >= wf.stepCount)
            return R;
        const auto &entry = wf.steps[step];

        // 2) Dispatch by layout type
        if (isListPopup(entry.mode))
        {
            // pick voice vs project list
            size_t n;
            switch (entry.mode)
            {
            case PopupMode::LoadVoiceList:
            case PopupMode::SaveVoiceList:
                n = popupState.listItems.size();
                break;
            default:
                n = popupState.listItems.size();
                break;
            }
            uint8_t max = n > 0 ? uint8_t(n - 1) : 0;
            R[0] = {0, max};
        }
        else if (isInputPopup(entry.mode))
        {
            // all four encoders cycle through the alphabet
            for (int i = 0; i < 4; ++i)
            {
                R[i] = {0, uint8_t(nameAlphabetSize - 1)};
            }
        }
        // else confirmation or others: leave all at {0,0}

        return R;
    }

} // namespace menu
