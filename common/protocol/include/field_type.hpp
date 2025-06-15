#pragma once
#include <cstdint>

namespace protocol

{

    // type of control
    enum class FieldType
    {
        Range,
        Options
    };

    struct FieldInfo
    {
        const char *label;
        FieldType type;
        int16_t min, max;
        const char *const *opts;
        uint8_t optCount;
        int16_t defaultValue;
        int16_t increment = 1;
    };

    // option lists
    static constexpr const char *noYes[] = {"Off", "On"};

} // namespace protocol
