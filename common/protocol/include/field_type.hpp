#pragma once
#include <cstdint>

namespace protocol

{

    // type of control
    enum class FieldType
    {
        Range,
        Options,
        Autosave
    };

    struct FieldInfo
    {
        const char *label;
        FieldType type;
        int min, max;
        const char *const *opts;
        uint8_t optCount;
    };

    // option lists
    static constexpr const char *yesNo[] = {"Off", "On"};

} // namespace protocol
