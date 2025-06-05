#pragma once
#include <cstdint>
#include "field_type.hpp"

namespace protocol
{
    enum class EnvelopeField : uint8_t
    {
        A,
        D,
        S,
        R,
        _Count
    };

    // envelope fields
    static constexpr FieldInfo envInfo[] = {
        {"A", FieldType::Range, 0, 31, nullptr, 0},
        {"D", FieldType::Range, 0, 31, nullptr, 0},
        {"S", FieldType::Range, 0, 31, nullptr, 0},
        {"R", FieldType::Range, 0, 31, nullptr, 0},
    };
}
