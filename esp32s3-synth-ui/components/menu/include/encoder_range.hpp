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

    /// Knob ranges when you’re browsing a list of items
    inline std::array<EncoderRange, 4> getPopupRangesList(const PopupState &st)
    {
        std::array<EncoderRange, 4> R{};

        // only knob 0 is used here:
        uint8_t maxSlot = st.listItems.empty()
                              ? 0
                              : static_cast<uint8_t>(st.listItems.size() - 1);
        R[0].min = 0;
        R[0].max = maxSlot;
        R[0].value = std::clamp<uint8_t>(st.slotIndex, 0, maxSlot);

        // clear knobs 1–3
        for (int k = 1; k < 4; ++k)
            R[k] = EncoderRange{0, 0, 0};

        return R;
    }

    /// Knob ranges when you’re renaming (4-char buffer + alphabet)
    inline std::array<EncoderRange, 4> getPopupRangesRename(const PopupState &st)
    {
        std::array<EncoderRange, 4> R{};

        // 0: choose which character (0..3)
        constexpr uint8_t MAX_CHAR_POS = sizeof(st.editName) / sizeof(*st.editName) - 1;
        R[0].min = 0;
        R[0].max = MAX_CHAR_POS;
        R[0].value = std::clamp<uint8_t>(st.stepIndex, 0, MAX_CHAR_POS);

        // 1: choose its value from nameAlphabet[]
        constexpr size_t ALPHA_LEN = sizeof(nameAlphabet) - 1;
        char cur = st.editName[R[0].value];
        auto it = std::find(nameAlphabet, nameAlphabet + ALPHA_LEN, cur);
        uint8_t idx = (it != nameAlphabet + ALPHA_LEN) ? static_cast<uint8_t>(it - nameAlphabet) : 0;
        R[1].min = 0;
        R[1].max = static_cast<uint8_t>(ALPHA_LEN - 1);
        R[1].value = idx;

        // clear knobs 2–3
        for (int k = 2; k < 4; ++k)
            R[k] = EncoderRange{0, 0, 0};

        return R;
    }

    /// Knob ranges when you’re on a yes/no confirmation
    inline std::array<EncoderRange, 4> getPopupRangesConfirm(const PopupState &st)
    {
        std::array<EncoderRange, 4> R{};

        // only knob 0 matters: yes/no
        R[0].min = 0;
        R[0].max = 1;
        R[0].value = std::clamp<uint8_t>(st.slotIndex, 0, 1);

        // clear knobs 1–3
        for (int k = 1; k < 4; ++k)
            R[k] = EncoderRange{0, 0, 0};

        return R;
    }

    /// Dispatch based on which array the current PopupMode belongs to
    inline std::array<EncoderRange, 4> getEncoderRangesPopup(const PopupState &st)
    {
        PopupMode mode = getCurrentPopupMode(st);

        if (std::find(listModes.begin(), listModes.end(), mode) != listModes.end())
            return getPopupRangesList(st);
        if (std::find(inputModes.begin(), inputModes.end(), mode) != inputModes.end())
            return getPopupRangesRename(st);
        if (std::find(confirmModes.begin(), confirmModes.end(), mode) != confirmModes.end())
            return getPopupRangesConfirm(st);

        // Fallback: no knobs
        return {};
    }

} // namespace menu
