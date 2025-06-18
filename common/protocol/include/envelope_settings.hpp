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
        static constexpr float MIN_BEAT_LENGTH = 0.001; // sharp attack
        static constexpr float MAX_BEAT_LENGTH = 64.0f;    // 8 beats
    }

    // envelope fields
    static constexpr FieldInfo envInfo[] = {
        {
            .label = "A",
            .type = FieldType::Range,
            .min = 0,
            .max = envelope::MAX,
            .opts = nullptr,
            .optCount = 0,
            .defaultValue = 1,
            .increment = 1,
        },
        {
            .label = "D",
            .type = FieldType::Range,
            .min = 0,
            .max = envelope::MAX,
            .opts = nullptr,
            .optCount = 0,
            .defaultValue = 1,
            .increment = 1,
        },
        {
            .label = "S",
            .type = FieldType::Range,
            .min = 0,
            .max = envelope::MAX,
            .opts = nullptr,
            .optCount = 0,
            .defaultValue = envelope::MAX,
            .increment = 1,
        },
        {
            .label = "R",
            .type = FieldType::Range,
            .min = 0,
            .max = envelope::MAX,
            .opts = nullptr,
            .optCount = 0,
            .defaultValue = 1,
            .increment = 1,
        },
    };

}
