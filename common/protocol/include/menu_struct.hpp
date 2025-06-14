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
#include "global_settings.hpp"
#include "channel_settings.hpp"

namespace protocol
{

    enum class AppMode : uint8_t
    {
        Loading, // page/voice selector
        MenuList, // page/voice selector
        Page,     // parameter editing
        Popup     // load/save dialog
    };
    // pages
    enum class Page : uint8_t
    {
        Channel = 0,
        Oscillator,
        Filter,
        Envelope,
        Tuning,
        PitchLFO,
        AmpLFO,
        FilterCLFO,
        FilterRLFO,
        Global, ///< Global BPM settings page
        _Count
    };



    // somewhere in your menu namespace, after the Page & *Field enums:
    static constexpr std::array<uint8_t, static_cast<size_t>(Page::_Count)> fieldsPerPage = {
        // Channels page
        static_cast<uint8_t>(ChannelField::_Count),
        // Oscillator page
        static_cast<uint8_t>(OscillatorField::_Count),
        // Filter page
        static_cast<uint8_t>(FilterField::_Count),
        // Envelope page
        static_cast<uint8_t>(EnvelopeField::_Count),
        // Tuning page
        static_cast<uint8_t>(TuningField::_Count),
        // PitchLFO page (same as Filter LFO)
        static_cast<uint8_t>(LFOField::_Count),
        // Amp LFO page
        static_cast<uint8_t>(LFOField::_Count),
        // FilterC LFO page
        static_cast<uint8_t>(LFOField::_Count),
        // FilterR LFO page
        static_cast<uint8_t>(LFOField::_Count),
        // Global BPM page
        static_cast<uint8_t>(GlobalField::_Count),
    };



    struct PageInfo
    {
        const char *title;
        const FieldInfo *fields;
        uint8_t fieldCount;
    };

    static constexpr PageInfo menuPages[] = {
        {"Vol/Channel", channelInfo, sizeof(channelInfo) / sizeof(FieldInfo)},
        {"Oscillator", oscInfo, sizeof(oscInfo) / sizeof(FieldInfo)},
        {"Filter", filterInfo, sizeof(filterInfo) / sizeof(FieldInfo)},
        {"Envelope", envInfo, sizeof(envInfo) / sizeof(FieldInfo)},
        {"Tuning", tuningInfo, sizeof(tuningInfo) / sizeof(FieldInfo)},
        {"Pitch LFO", lfoInfo, sizeof(lfoInfo) / sizeof(FieldInfo)},
        {"Amp LFO", lfoInfo, sizeof(lfoInfo) / sizeof(FieldInfo)},
        {"FilterC LFO", lfoInfo, sizeof(lfoInfo) / sizeof(FieldInfo)},
        {"FilterR LFO", lfoInfo, sizeof(lfoInfo) / sizeof(FieldInfo)},
        {"BPM", bpmInfo, sizeof(bpmInfo) / sizeof(FieldInfo)},
    };

    static constexpr uint8_t MAX_FIELDS = 4;
    static constexpr size_t PAGE_COUNT = static_cast<size_t>(Page::_Count);
    static constexpr size_t GLOBAL_PAGE_COUNT = 1;
    static constexpr size_t VOICE_PAGE_COUNT = PAGE_COUNT - GLOBAL_PAGE_COUNT;

} // namespace menu
