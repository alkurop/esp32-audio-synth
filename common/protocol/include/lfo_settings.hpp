#pragma once
#include "field_type.hpp"

namespace protocol
{

    constexpr const uint8_t LFO_DEPTH_MAX = 127;

    enum class LfoSubdivision : uint8_t
    {
        Double = 0,    // "2/1"
        Whole,         // "1/1"
        Half,          // "1/2"
        Quarter,       // "1/4"
        DottedQuarter, // "3/8"
        Eighth,        // "1/8"
        Sixteenth,     // "1/16"
        ThirtySecond,  // "1/32"
    };

    static constexpr float beatsPerCycleMap[] = {
        2.0f,    // Double (2 beats per cycle)
        1.0f,    // Whole
        0.5f,    // Half
        0.25f,   // Quarter
        0.375f,  // Dotted Quarter (3/8)
        0.125f,  // Eighth
        0.0625f, // Sixteenth
        0.03125f // Thirty-second
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
        Subdiv,
        Form,
        Depth,
        _Count
    };

    static constexpr FieldInfo lfoInfo[] = {
        {
            .label = "Subd",
            .type = FieldType::Options,
            .min = 0,
            .max = 0,
            .opts = subdivisions,
            .optCount = 8,
            .defaultValue = 0,
            .increment = 1,
        },
        {
            .label = "Form",
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
