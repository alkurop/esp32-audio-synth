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
        Resonance
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
        {"Type", FieldType::Options, 0, 0, filtTypes, 4},
        {"Cuto", FieldType::Range, 0, MAX_CUTOFF_RAW, nullptr, 0, 4},
        {"Res", FieldType::Range, 0, MAX_CUTOFF_RAW, nullptr, 0, 4},
    };

}
