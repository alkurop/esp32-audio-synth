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
        const PopupState &popupState, const ParamStore *paramStore)
    {
        std::array<EncoderRange, 4> R;
        switch (popupState.mode)
        {
        case PopupMode::LoadVoiceList: // fallthrough
        case PopupMode::SaveVoiceList:
            R[0] = EncoderRange{0, static_cast<uint8_t>(paramStore->listVoiceNames().size() - 1)};
            for (int i = 1; i < 4; ++i)
            {
                R[i] = EncoderRange{0, 0};
            }
            break;
        case PopupMode::SaveProjectRename: // fallthrough
        case PopupMode::SaveVoiceRename:
            for (int i = 0; i < 4; ++i)
            {
                R[i] = EncoderRange{0, static_cast<uint8_t>(nameAlphabetSize - 1)};
            }
            break;

        case PopupMode::LoadProjectList: // fallthrough
        case PopupMode::SaveProjectList:
            R[0] = EncoderRange{0, static_cast<uint8_t>(paramStore->listProjectNames().size() - 1)};
            for (int i = 1; i < 4; ++i)
            {
                R[i] = EncoderRange{0, 0};
            }
            break;
        default:
            for (int i = 0; i < 4; ++i)
            {
                R[i] = EncoderRange{0, 0};
            }
            break;
        }
        return R;
    }

} // namespace menu
