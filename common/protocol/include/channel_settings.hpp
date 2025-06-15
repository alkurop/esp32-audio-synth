#pragma once
#include "field_type.hpp"

namespace protocol
{

    namespace channel
    {
        constexpr uint8_t CHANNEL_MAX = 15;
    }

    // fields per page
    enum class ChannelField : uint8_t
    {
        Chan,
        Vol,
        _Count
    };

    namespace voice
    {
        constexpr uint8_t VOL_MAX = 31;
        constexpr float MIN_DB = -60.0f;
        constexpr float MAX_DB = 0.0f;
        constexpr uint8_t MAX_POLYPHONY = 8;

        struct SmoothedValue
        {
            float current = 0.0f; // the value we actually output each sample
            float target = 0.0f;  // the “destination” value we want to reach
            float alpha = 0.002f; // smoothing coefficient (controls ramp speed)

            // Call this whenever you want to change the parameter (e.g. volume)
            void setTarget(float newTarget)
            {
                target = newTarget;
            }

            // Call this once per audio sample to step `current` toward `target`.
            float next()
            {
                current += alpha * (target - current);
                return current;
            }
        };

        struct VolumeSettings
        {
            uint8_t volume;
            SmoothedValue gain_smoothed;
        };

        struct AudioConfig
        {
            uint32_t sample_rate; // in Hz
            size_t max_polyphony; // number of simultaneous sounds;
        };

        struct PitchSettings
        {
            int16_t fine_tuning;
            int16_t transpose_semitones;
            int16_t transpose_octave;
        };

    }
    // channel fields
    static constexpr FieldInfo channelInfo[] = {
        {
            .label = "Chan",
            .type = FieldType::Range,
            .min = 0,
            .max = channel::CHANNEL_MAX,
            .opts = nullptr,
            .optCount = 0,
            .defaultValue = 0,
            .increment = 1,
        },
        {
            .label = "Vol",
            .type = FieldType::Range,
            .min = 0,
            .max = voice::VOL_MAX,
            .opts = nullptr,
            .optCount = 0,
            .defaultValue = 0,
            .increment = 1,
        },
    };
}
