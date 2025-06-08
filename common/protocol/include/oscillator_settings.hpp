#pragma once
#include "field_type.hpp"

namespace protocol
{

    enum OscillatorShape
    {
        Sine,
        Saw,
        Square,
        Tri,
        Noise,
        _Count
    };
    // oscillator fields
    static constexpr const char *oscShapes[] = {"Sine", "Saw", "Square", "Tri", "Noise"};
    static constexpr FieldInfo oscInfo[] = {
        {"Shape", FieldType::Options, 0, 0, oscShapes, 5},
        {"Morph", FieldType::Range, 0, 31, nullptr, 0},
        {"PWM", FieldType::Range, 0, 31, nullptr, 0},
        {"Sync", FieldType::Options, 0, 0, yesNo, 2},
    };

    enum class OscillatorField : uint8_t
    {
        Shape,
        Morph,
        PWM,
        Sync,
        _Count
    };

};
