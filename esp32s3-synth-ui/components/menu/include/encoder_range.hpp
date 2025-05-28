#pragma once
#include <array>
#include "menu_struct.hpp"
#include "param_store.hpp"

#define MENU_POSITION_LIST 0
#define MENU_POSITION_VOICE 1
#define MENU_POSITION_CH 2
#define MENU_POSITION_VOL 3

namespace menu
{

    static inline std::array<EncoderRange, 4> getEncoderRangesMenuList(
        uint8_t voiceCount,
        const MenuState &state)
    {
        std::array<EncoderRange, 4> R;

        // page selector
        R[MENU_POSITION_LIST] = {
            min : 0,
            max : static_cast<uint8_t>(menuItemCnt - 1),
            value : state.menuItemIndex
        };

        // voice selector
        R[MENU_POSITION_VOICE] = {
            min : 0,
            max : static_cast<uint8_t>(voiceCount - 1),
            value : state.volume
        };

        // channel on Main List (always numeric)
        {
            const auto &fi = menuPages[size_t(Page::Channel)].fields[size_t(ChannelField::Chan)];
            R[MENU_POSITION_CH] = {
                min : static_cast<int16_t>(fi.min),
                max : static_cast<int16_t>(fi.max),
                value : state.channel
            };
        }

        // volume on Main List (always numeric)
        {
            const auto &fi = menuPages[size_t(Page::Channel)].fields[size_t(ChannelField::Vol)];
            R[MENU_POSITION_VOL] = {
                min : static_cast<int16_t>(fi.min),
                max : static_cast<int16_t>(fi.max),
                value : state.volume
            };
        }

        return R;
    }

    /// Edit‐page ranges: knob0–3 map directly to the current page’s fields
    static inline std::array<EncoderRange, 4> getEncoderRangesPage(
        const Page &page,
        const MenuState &state)
    {
        std::array<EncoderRange, 4> R;
        const auto &pi = menuPages[size_t(page)];

        for (int k = 0; k < 4; ++k)
        {
            if (k < pi.fieldCount)
            {
                const auto &fi = pi.fields[k];
                if (fi.type == FieldType::Range)
                    R[k] = EncoderRange{
                        min : static_cast<int16_t>(fi.min),
                        max : static_cast<int16_t>(fi.max),
                        value : state.fieldValues[k]
                    };
                else
                    R[k] = EncoderRange{
                        min : 0,
                        max : static_cast<int16_t>(fi.optCount - 1),
                        value : state.fieldValues[k]
                    };
            }
            else
            {
                R[k] = EncoderRange{min : 0, max : 0, value : 0};
            }
        }
        return R;
    }

    static inline std::array<EncoderRange, 4> getEncoderRangesPopup(
        const PopupState &st)
    {
        std::array<EncoderRange, 4> R{};

        // 0) Step selector
        {
            size_t wf = size_t(st.workflowIndex);
            const auto &wk = (wf < workflowCnt ? popupWorkflows[wf] : popupWorkflows[0]);
            R[0] = {0,
                    static_cast<uint8_t>(wk.stepCount - 1),
                    st.stepIndex};
        }

        // 1) Slot selector
        {
            uint8_t maxSlot = st.listItems.empty() ? 0 : static_cast<uint8_t>(st.listItems.size() - 1);
            R[1] = {min : 0,
                    max : maxSlot,
                    value : std::clamp<uint8_t>(st.slotIndex, 0, maxSlot)};
        }

        // 2) Character‐index selector (0..3 for a 4‐char name)
        {
            constexpr uint8_t maxCharPos = sizeof(st.editName) / sizeof(*st.editName) - 1;
            // You might want a separate `charIndex` in PopupState; if not, reuse slotIndex
            uint8_t pos = std::clamp<uint8_t>(st.slotIndex, 0, maxCharPos);
            R[2] = {min : 0, max : maxCharPos, value : pos};
        }

        // 3) Character‐value selector using nameAlphabet[]
        {
            constexpr size_t alphaLen = sizeof(nameAlphabet) - 1; // minus trailing '\0'

            // Determine current letter index
            char cur = st.editName[std::clamp<uint8_t>(st.slotIndex, 0, alphaLen - 1)];
            auto it = std::find(nameAlphabet,
                                nameAlphabet + alphaLen,
                                cur);
            uint8_t idx = (it != nameAlphabet + alphaLen)
                              ? static_cast<uint8_t>(it - nameAlphabet)
                              : 0;

            R[3] = {0,
                    static_cast<uint8_t>(alphaLen - 1),
                    idx};
        }

        return R;
    }

} // namespace menu
