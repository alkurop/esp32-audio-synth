#pragma once
#include "field_type.hpp"

namespace protocol
{

    constexpr const uint8_t LFO_DEPTH_MAX = 127;
    
    enum class LfoSubdivision : uint8_t
    {
        Double = 128,       // "2/1"
        Whole = 64,         // "1/1"
        Half = 32,          // "1/2"
        Quarter = 16,       // "1/4"
        DottedQuarter = 24, // "3/8"
        Eighth = 8,         // "1/8"
        Sixteenth = 4,      // "1/16"
        ThirtySecond = 2,   // "1/32"
    };

    // New: which waveform the LFO outputs
    enum class LfoWaveform : uint8_t
    {
        Sine = 0,
        Triangle,
        Sawtooth,
        Pulse,
    };

    static constexpr const char *subdivisions[] =
        {"2/1", "1/1", "1/2", "1/4", "3/8", "1/8", "1/16", "1/32"};

    static constexpr const char *form[] =
        {"Sine", "Triangle", "Sawtooth", "Pulse"};

    enum class LFOField : uint8_t
    {
        Form,
        Subdiv,
        Depth,
        _Count
    };

    static constexpr FieldInfo lfoInfo[] = {
        {
            .label = "Form",
            .type = FieldType::Options,
            .min = 0,
            .max = 0,
            .opts = subdivisions,
            .optCount = 8,
            .defaultValue = 0,
            .increment = 1,
        },
        {
            .label = "Subd",
            .type = FieldType::Options,
            .min = 0,
            .max = 0,
            .opts = form,
            .optCount = 4,
            .defaultValue = 0,
            .increment = 1,
        },
        {
            .label = "Dept",
            .type = FieldType::Range,
            .min = 0,
            .max = LFO_DEPTH_MAX,
            .opts = nullptr,
            .optCount = 0,
            .defaultValue = 4,
            .increment = 1,
        },
    };

}
