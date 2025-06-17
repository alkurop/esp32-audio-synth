#pragma once
#include "field_type.hpp"

namespace protocol
{
    enum class TuningField : uint8_t
    {
        Octave,
        Semitone,
        FineTune,
        _Count
    };

    // tuning fields
    static constexpr FieldInfo tuningInfo[] = {
        {
            .label = "Oct",
            .type = FieldType::Range,
            .min = -2,
            .max = 2,
            .opts = nullptr,
            .optCount = 0,
            .defaultValue = 0,
            .increment = 1,
        },
        {
            .label = "Semi",
            .type = FieldType::Range,
            .min = -50,
            .max = 50,
            .opts = nullptr,
            .optCount = 0,
            .defaultValue = 0,
            .increment = 1,
        },
        {
            .label = "Fine",
            .type = FieldType::Range,
            .min = -50,
            .max = 50,
            .opts = nullptr,
            .optCount = 0,
            .defaultValue = 0,
            .increment = 1,
        },
    };

}
