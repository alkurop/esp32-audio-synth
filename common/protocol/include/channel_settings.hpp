#pragma once
#include "field_type.hpp"

namespace protocol
{
    // channel fields
    static constexpr FieldInfo channelInfo[] = {
        {"Chan", FieldType::Range, 0, 15, nullptr, 0},
        {"Vol", FieldType::Range, 0, 15, nullptr, 0},
    };

    // fields per page
    enum class ChannelField : uint8_t
    {
        Chan,
        Vol,
        _Count
    };

}
