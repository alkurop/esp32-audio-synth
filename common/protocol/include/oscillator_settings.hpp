#pragma once
#include "field_type.hpp"

namespace protocol
{
    enum class OscillatorField : uint8_t
    {
        Shape,
        PWM,
        Sync,
        _Count
    };
    
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
        {
            .label = "Shape",
            .type = FieldType::Options,
            .min = 0,
            .max = 0,
            .opts = oscShapes,
            .optCount = 5,
            .defaultValue = 0,
            .increment = 1,
        },
        {
            .label = "PWM",
            .type = FieldType::Range,
            .min = 0,
            .max = OSCILLATOR_PWM_MAX,
            .opts = nullptr,
            .optCount = 0,
            .defaultValue = 0,
            .increment = 1,
        },
        
    };

};
