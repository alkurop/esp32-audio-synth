#pragma once
#include "field_type.hpp"

namespace protocol
{
    // BpmFields
    static constexpr FieldInfo bpmInfo[] = {
        {"Sync", FieldType::Options, 0, 0, yesNo, 2},
        {"BPM", FieldType::Range, 30, 300, nullptr, 0},
    };

    enum class GlobalField : uint8_t
    {
        SyncMode,
        ManualBPM,
        _Count
    };

}
