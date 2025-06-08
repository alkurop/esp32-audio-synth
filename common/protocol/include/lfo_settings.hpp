#pragma once
#include "field_type.hpp"

namespace protocol
{

    enum class LfoSubdivision : uint8_t
    {
        Double = 128,       // "2/1"
        Whole = 64,         // "1/1"
        Half = 32,          // "1/2"
        Quarter = 16,       // "1/4"
        DottedQuarter = 24, // "3/8"
        Eighth = 8,         // "1/8"
        Sixteenth = 4,      // "1/16"
        ThirtySecond = 2    // "1/32"
    };

    // New: which waveform the LFO outputs
    enum class LfoWaveform : uint8_t
    {
        Sine = 0,
        Triangle,
        Sawtooth,
        Pulse
    };

    static constexpr const char *subdivisions[] =
        {"2/1", "1/1", "1/2", "1/4", "3/8", "1/8", "1/16", "1/32"};

    static constexpr const char *form[] =
        {"Sine", "Triangle", "Sawtooth", "Pulse"};

    static constexpr FieldInfo lfoInfo[] = {
        {"Form", FieldType::Options, 0, 0, subdivisions, 8},
        {"Subd", FieldType::Options, 0, 0, form, 4},
        {"Dept", FieldType::Range, 0, 127, nullptr, 0, 4},
    };

    enum class LFOField : uint8_t
    {
        Form,
        Subdiv,
        Depth,
        _Count
    };
}
