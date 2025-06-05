#pragma once
#include "field_type.hpp"

namespace protocol
{
    enum class FilterField : uint8_t
    {
        Type,
        Cut,
        Res,
        _Count
    };

    // filter fields
    static constexpr const char *filtTypes[] = {"LP12", "HP12", "BP12", "Notch"};
    static constexpr FieldInfo filterInfo[] = {
        {"Type", FieldType::Options, 0, 0, filtTypes, 4},
        {"Cut", FieldType::Range, 0, 31, nullptr, 0},
        {"Res", FieldType::Range, 0, 31, nullptr, 0},
    };

}
