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

    }
    // channel fields
    static constexpr FieldInfo channelInfo[] = {
        {"Chan", FieldType::Range, 0, channel::CHANNEL_MAX, nullptr, 0},
        {"Vol", FieldType::Range, 0, voice::VOL_MAX, nullptr, 0},
    };
}
