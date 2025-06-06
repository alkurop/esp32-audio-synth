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

    namespace envelope
    {
        static constexpr uint8_t MAX = 31;
    }

    // envelope fields
    static constexpr FieldInfo envInfo[] = {
        {"A", FieldType::Range, 0, envelope::MAX, nullptr, 0},
        {"D", FieldType::Range, 0, envelope::MAX, nullptr, 0},
        {"S", FieldType::Range, 0, envelope::MAX, nullptr, 0},
        {"R", FieldType::Range, 0, envelope::MAX, nullptr, 0},
    };


}
