#pragma once

#include <cmath>
#include <cstdint>

namespace sound_module
{
    struct Sound
    {
        bool active = false;
        uint8_t midi_note = 0; // store note for note_off
        float phase = 0.0f;
        float phase_increment = 0.0f;
        float velocity = 1.0f;

        // Receive midi_note, frequency, and velocity
        inline void note_on(uint8_t midi_note_in,
                            float frequency,
                            float velocity_in,
                            float sample_rate)
        {
            midi_note = midi_note_in;
            velocity = velocity_in;
            phase = 0.0f;
            phase_increment = frequency / sample_rate;
            active = true;
        }

        inline void note_off()
        {
            active = false;
        }

        inline float get_sample(float /*sample_rate*/)
        {
            if (!active)
                return 0.0f;

            float sample = sinf(2.0f * M_PI * phase) * velocity;
            phase += phase_increment;
            if (phase >= 1.0f)
                phase -= 1.0f;

            return sample;
        }
    };
} // namespace sound_module
