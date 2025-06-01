#pragma once
#include <cstdint>
#include <array>
#include <vector>
#include <functional>

namespace protocol
{

    enum class AppMode : uint8_t
    {
        MenuList, // page/voice selector
        Page,     // parameter editing
        Popup     // load/save dialog
    };
    // pages
    enum class Page : uint8_t
    {
        Channel,
        Oscillator,
        Filter,
        Envelope,
        Tuning,
        FilterLFO,
        DetuneLFO,
        Global, ///< Global BPM settings page
        _Count
    };

    // fields per page
    enum class ChannelField : uint8_t
    {
        Chan,
        Vol,
        _Count
    };
    enum class OscillatorField : uint8_t
    {
        Shape,
        Morph,
        PWM,
        Sync,
        _Count
    };

    enum class FilterField : uint8_t
    {
        Type,
        Cut,
        Res,
        _Count
    };
    enum class EnvField : uint8_t
    {
        A,
        D,
        S,
        R,
        _Count
    };
    enum class TuningField : uint8_t
    {
        Octave,
        Semitone,
        FineTune,
        _Count
    };
    enum class LFOField : uint8_t
    {
        RateMode,
        Rate,
        Subdiv,
        Depth,
        _Count
    };
    enum class GlobalField : uint8_t
    {
        SyncMode,
        ManualBPM,
        _Count
    };

    // type of control
    enum class FieldType
    {
        Range,
        Options,
        Autosave
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
        static_cast<uint8_t>(EnvField::_Count),
        // Tuning page
        static_cast<uint8_t>(TuningField::_Count),
        // Filter LFO page
        static_cast<uint8_t>(LFOField::_Count),
        // Detune LFO page (same as Filter LFO)
        static_cast<uint8_t>(LFOField::_Count),
        // Global BPM page
        static_cast<uint8_t>(GlobalField::_Count),
    };

    struct FieldInfo
    {
        const char *label;
        FieldType type;
        int min, max;
        const char *const *opts;
        uint8_t optCount;
    };

    // option lists
    static constexpr const char *yesNo[] = {"Off", "On"};

    // channel fields
    static constexpr FieldInfo channelInfo[] = {
        {"Chan", FieldType::Range, 0, 15, nullptr, 0},
        {"Vol", FieldType::Range, 0, 15, nullptr, 0},
    };

    // oscillator fields
    static constexpr const char *oscShapes[] = {"Sine", "Saw", "Square", "Tri"};
    static constexpr FieldInfo oscInfo[] = {
        {"Shape", FieldType::Options, 0, 0, oscShapes, 4},
        {"Morph", FieldType::Range, 0, 31, nullptr, 0},
        {"PWM", FieldType::Range, 0, 31, nullptr, 0},
        {"Sync", FieldType::Options, 0, 0, yesNo, 2},
    };

    // filter fields
    static constexpr const char *filtTypes[] = {"LP12", "HP12", "BP12", "Notch"};
    static constexpr FieldInfo filterInfo[] = {
        {"Type", FieldType::Options, 0, 0, filtTypes, 4},
        {"Cut", FieldType::Range, 0, 31, nullptr, 0},
        {"Res", FieldType::Range, 0, 31, nullptr, 0},
    };

    // envelope fields
    static constexpr FieldInfo envInfo[] = {
        {"A", FieldType::Range, 0, 31, nullptr, 0},
        {"D", FieldType::Range, 0, 31, nullptr, 0},
        {"S", FieldType::Range, 0, 31, nullptr, 0},
        {"R", FieldType::Range, 0, 31, nullptr, 0},
    };

    // tuning fields
    static constexpr FieldInfo tuningInfo[] = {
        {"Oct", FieldType::Range, -2, 2, nullptr, 0},
        {"Semi", FieldType::Range, -12, 12, nullptr, 0},
        {"Fine", FieldType::Range, -50, 50, nullptr, 0},
    };

    // LFO fields
    static constexpr const char *subdivisions[] = {"1/1", "1/2", "1/4", "1/8", "1/16", "1/32"};
    static constexpr const char *rateModes[] = {"Free", "Sync"};
    static constexpr FieldInfo lfoInfo[] = {
        {"Mode", FieldType::Options, 0, 0, rateModes, 2},
        {"Rate", FieldType::Range, 0, 31, nullptr, 0},
        {"Subd", FieldType::Options, 0, 0, subdivisions, 6},
        {"Dept", FieldType::Range, 0, 31, nullptr, 0},
    };

    // BpmFields
    static constexpr FieldInfo bpmInfo[] = {
        {"Sync", FieldType::Options, 0, 0, yesNo, 2},
        {"BPM", FieldType::Range, 30, 300, nullptr, 0},
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
        {"Filter LFO", lfoInfo, sizeof(lfoInfo) / sizeof(FieldInfo)},
        {"Detune LFO", lfoInfo, sizeof(lfoInfo) / sizeof(FieldInfo)},
        {"BPM", bpmInfo, sizeof(bpmInfo) / sizeof(FieldInfo)},
    };

    static constexpr uint8_t MAX_FIELDS = 4;
    static constexpr size_t PAGE_COUNT = static_cast<size_t>(Page::_Count);
    static constexpr size_t GLOBAL_PAGE_COUNT = 1;
    static constexpr size_t VOICE_PAGE_COUNT = PAGE_COUNT - GLOBAL_PAGE_COUNT;

    

} // namespace menu
