#pragma once
#include "field_type.hpp"

namespace protocol
{

    static constexpr int AUTOSAVE_INTERVAL_MS = (1 * 20 * 1000); // 20 seconds

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

    namespace global_settings
    {
        constexpr const uint16_t INITIAL_BMP = 120;
    };

}
