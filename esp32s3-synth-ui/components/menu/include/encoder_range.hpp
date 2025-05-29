#pragma once
#include <array>
#include <algorithm> // for std::clamp and std::find
#include "menu_struct.hpp"
#include "param_store.hpp"

#define KNOB_COUNT 4

#define MENU_POSITION_LIST 0
#define MENU_POSITION_VOICE 1
#define MENU_POSITION_CH 2
#define MENU_POSITION_VOL 3

namespace menu
{
    static_assert(KNOB_COUNT > 0, "KNOB_COUNT must be > 0");

    static inline std::array<EncoderRange, KNOB_COUNT> getEncoderRangesMenuList(
        uint8_t voiceCount,
        const MenuState &state)
    {
        std::array<EncoderRange, KNOB_COUNT> R;

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
        std::array<EncoderRange, KNOB_COUNT> R;
        const auto &pi = menuPages[size_t(page)];

        for (int k = 0; k < KNOB_COUNT; ++k)
        {
            if (k < pi.fieldCount)
            {
                const auto &fi = pi.fields[k];
                if (fi.type == FieldType::Range)
                    R[k] = EncoderRange{
                        .min = static_cast<int16_t>(fi.min),
                        .max = static_cast<int16_t>(fi.max),
                        .value = state.fieldValues[k]};
                else
                    R[k] = EncoderRange{
                        .min = 0,
                        .max = static_cast<int16_t>(fi.optCount - 1),
                        .value = state.fieldValues[k]};
            }
            else
            {
                R[k] = EncoderRange{.min = 0, .max = 0, .value = 0};
            }
        }
        return R;
    }

    /// Knob ranges when you’re browsing a list of items
    inline std::array<EncoderRange, KNOB_COUNT> getPopupRangesList(const PopupState &st)
    {
        std::array<EncoderRange, KNOB_COUNT> R{};

        // clear all knobs
        for (int k = 0; k < KNOB_COUNT; ++k)
            R[k] = {.min = 0, .max = 0, .value = 0};

        // only knob 0 is used here
        uint8_t maxSlot = st.listItems.empty()
                              ? 0
                              : static_cast<uint8_t>(st.listItems.size() - 1);
        R[0] = {.min = 0,
                .max = maxSlot,
                .value = std::clamp<uint8_t>(st.slotIndex, 0, maxSlot)};

        return R;
    }

    /// Knob ranges when you’re renaming (one knob per character)
    inline std::array<EncoderRange, KNOB_COUNT> getPopupRangesRename(const PopupState &st)
    {
        std::array<EncoderRange, KNOB_COUNT> R{};
        constexpr size_t ALPHA_LEN = sizeof(nameAlphabet) - 1;
        constexpr uint8_t MAX_IDX = static_cast<uint8_t>(ALPHA_LEN - 1);

        // each knob edits one letter
        for (int k = 0; k < KNOB_COUNT; ++k)
        {
            char cur = st.editName[k];
            auto it = std::find(nameAlphabet, nameAlphabet + ALPHA_LEN, cur);
            uint8_t val = (it != nameAlphabet + ALPHA_LEN)
                              ? static_cast<uint8_t>(it - nameAlphabet)
                              : 0;
            R[k] = {.min = 0,
                    .max = MAX_IDX,
                    .value = val};
        }

        return R;
    }

    /// Knob ranges when you’re on a yes/no confirmation
    inline std::array<EncoderRange, KNOB_COUNT> getPopupRangesConfirm(const PopupState &st)
    {
        std::array<EncoderRange, KNOB_COUNT> R{};

        // clear all knobs
        for (int k = 0; k < KNOB_COUNT; ++k)
            R[k] = {.min = 0, .max = 0, .value = 0};

        // only knob 0 matters
        R[0] = {.min = 0,
                .max = 1,
                .value = std::clamp<uint8_t>(st.slotIndex, 0, 1)};

        return R;
    }

    inline std::array<EncoderRange, KNOB_COUNT> getEncoderRangesPopup(const PopupState &st)
    {
        PopupMode mode = getCurrentPopupMode(st);

        if (std::find(listModes.begin(), listModes.end(), mode) != listModes.end())
            return getPopupRangesList(st);
        if (std::find(inputModes.begin(), inputModes.end(), mode) != inputModes.end())
            return getPopupRangesRename(st);
        if (std::find(confirmModes.begin(), confirmModes.end(), mode) != confirmModes.end())
            return getPopupRangesConfirm(st);

        // default no knobs
        std::array<EncoderRange, KNOB_COUNT> empty{};
        for (auto &e : empty)
            e = {.min = 0, .max = 0, .value = 0};
        return empty;
    }

} // namespace menu
