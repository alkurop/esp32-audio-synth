#pragma once
#include "field_type.hpp"

namespace protocol
{

    enum OscillatorShape
    {
        Sine,
        Tri,
        Square,
        Saw,
        Noise,
        _Count
    };
    constexpr const uint8_t OSCILLATOR_MORPH_MAX = 31;
    constexpr const uint8_t OSCILLATOR_PWM_MAX = 31;
    // oscillator fields
    static constexpr const char *oscShapes[] = {"Sine", "Tri", "Square", "Saw", "Noise"};
    static constexpr FieldInfo oscInfo[] = {
        {"Shape", FieldType::Options, 0, 0, oscShapes, 5},
        {"PWM", FieldType::Range, 0, OSCILLATOR_PWM_MAX, nullptr, 0},
        {"Sync", FieldType::Options, 0, 0, yesNo, 2},
    };

    enum class OscillatorField : uint8_t
    {
        Shape,
        PWM,
        Sync,
        _Count
    };

};
