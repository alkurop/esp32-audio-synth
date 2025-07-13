#pragma once
#include <cstdint>
#include <array>
#include <vector>
#include <functional>
#include "envelope_settings.hpp"
#include "lfo_settings.hpp"
#include "oscillator_settings.hpp"
#include "tuning_settings.hpp"
#include "filter_settings.hpp"
#include "bpm_settings.hpp"
#include "channel_settings.hpp"

namespace protocol
{

    enum class AppMode : uint8_t
    {
        Loading,  // page/voice selector
        MenuList, // page/voice selector
        Page,     // parameter editing
        Popup     // load/save dialog
    };
    // pages
    enum class Page : uint8_t
    {

        Oscillator = 0,
        Filter,
        Envelope,
        Tuning,
        PitchLFO,
        AmpLFO,
        VolChan,
        Bpm, ///< Global BPM settings page
        _Count
    };

    struct PageInfo
    {
        const char *title;
        const FieldInfo *fields;
        uint8_t fieldCount;
    };

    static constexpr PageInfo menuPages[] = {
        {"Oscillator", oscInfo, sizeof(oscInfo) / sizeof(FieldInfo)},
        {"Filter", filterInfo, sizeof(filterInfo) / sizeof(FieldInfo)},
        {"Envelope", envInfo, sizeof(envInfo) / sizeof(FieldInfo)},
        {"Tuning", tuningInfo, sizeof(tuningInfo) / sizeof(FieldInfo)},
        {"Pitch LFO", lfoInfo, sizeof(lfoInfo) / sizeof(FieldInfo)},
        {"Amp LFO", lfoInfo, sizeof(lfoInfo) / sizeof(FieldInfo)},
        {"Vol/Channel", channelInfo, sizeof(channelInfo) / sizeof(FieldInfo)},
        {"BPM", bpmInfo, sizeof(bpmInfo) / sizeof(FieldInfo)},
    };

    static constexpr uint8_t MAX_FIELDS = 4;
    static constexpr size_t PAGE_COUNT = static_cast<size_t>(Page::_Count);
    static constexpr size_t GLOBAL_PAGE_COUNT = 1;
    static constexpr size_t VOICE_PAGE_COUNT = PAGE_COUNT - GLOBAL_PAGE_COUNT;

} // namespace menu
