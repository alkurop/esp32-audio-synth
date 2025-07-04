#pragma once
#include "field_type.hpp"
#include "audio_config.hpp"

namespace protocol
{

    static constexpr int AUTOSAVE_INTERVAL_MS = (1 * 20 * 1000); // 20 seconds

    enum class GlobalField : uint8_t
    {
        SyncMode,
        ManualBPM,
        _Count
    };

       static constexpr FieldInfo bpmInfo[] = {
        {
            .label = "Sync",
            .type = FieldType::Options,
            .min = 0,
            .max = 0,
            .opts = noYes,
            .optCount = 2,
            .defaultValue = 0,
            .increment = 1,
        },
        {
            .label = "BPM",
            .type = FieldType::Range,
            .min = 30,
            .max = 300,
            .opts = nullptr,
            .optCount = 0,
            .defaultValue = BPM_DEFAULT,
            .increment = 1,
        },
    };

}
