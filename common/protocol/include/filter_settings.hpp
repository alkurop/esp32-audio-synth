#pragma once
#include "field_type.hpp"

namespace protocol
{
    // Maximum raw values for modulation parameters
    static constexpr uint8_t MAX_CUTOFF_RAW = 127;
    static constexpr uint8_t MAX_RESONANCE_RAW = 127;
    enum class FilterField : uint8_t
    {
        Type,
        Cutoff,
        Resonance,
        _Count
    };

    enum class FilterType : uint8_t
    {
        LP12 = 0,
        HP12,
        BP12,
        Notch
    };

    // filter fields
    static constexpr const char *filtTypes[] = {"LP12", "HP12", "BP12", "Notch"};
    static constexpr FieldInfo filterInfo[] = {
        {
            .label = "Type",
            .type = FieldType::Options,
            .min = 0,
            .max = 0,
            .opts = filtTypes,
            .optCount = 4,
            .defaultValue = 0,
            .increment = 1,
        },
        {
            .label = "Cuto",
            .type = FieldType::Range,
            .min = 0,
            .max = MAX_CUTOFF_RAW,
            .opts = nullptr,
            .optCount = 0,
            .defaultValue = 4,
            .increment = 1,
        },
        {
            .label = "Res",
            .type = FieldType::Range,
            .min = 0,
            .max = MAX_RESONANCE_RAW,
            .opts = nullptr,
            .optCount = 0,
            .defaultValue = 4,
            .increment = 1,
        },
    };
}
