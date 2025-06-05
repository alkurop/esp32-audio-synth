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
        {"Oct", FieldType::Range, -2, 2, nullptr, 0},
        {"Semi", FieldType::Range, -12, 12, nullptr, 0},
        {"Fine", FieldType::Range, -50, 50, nullptr, 0},
    };
}
