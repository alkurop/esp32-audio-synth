#pragma once
#include "field_type.hpp"

namespace protocol
{

    // LFO fields
    static constexpr const char *subdivisions[] = {"1/1", "1/2", "1/4", "1/8", "1/16", "1/32"};
    static constexpr const char *rateModes[] = {"Free", "Sync"};
    static constexpr FieldInfo lfoInfo[] = {
        {"Mode", FieldType::Options, 0, 0, rateModes, 2},
        {"Rate", FieldType::Range, 0, 31, nullptr, 0},
        {"Subd", FieldType::Options, 0, 0, subdivisions, 6},
        {"Dept", FieldType::Range, 0, 31, nullptr, 0},
    };

    enum class LFOField : uint8_t
    {
        RateMode,
        Rate,
        Subdiv,
        Depth,
        _Count
    };
}
