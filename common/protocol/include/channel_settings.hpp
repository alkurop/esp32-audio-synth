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

        struct OscillatorSettings
        {
            protocol::OscillatorShape shape;
            uint8_t pwm;
            bool syncOn;
        };
        struct EnvelopeSettings
        {
            uint8_t attack;
            uint8_t decay;
            uint8_t sustain;
            uint8_t release;
        };

        struct PitchSettings
        {
            int16_t fine_tuning;
            int16_t transpose_semitones;
            int16_t transpose_octave;
            int16_t totalTransposeCents = 0;
            float pitchRatio = 1.0;
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
