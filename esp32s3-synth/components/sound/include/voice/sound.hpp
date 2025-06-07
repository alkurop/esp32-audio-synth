#pragma once

#include <cmath>
#include <cstdint>

namespace sound_module
{
    struct Sound
    {
        bool active = false;
        uint8_t midi_note = 0;
        float base_frequency = 0.0f; // ← new
        float phase = 0.0f;
        float phase_increment = 0.0f;
        float velocity = 1.0f;

        inline void note_on(uint8_t midi_note_in,
                            float frequency,
                            float velocity_in,
                            float sample_rate)
        {
            midi_note = midi_note_in;
            base_frequency = frequency; // store raw base
            velocity = velocity_in;
            phase = 0.0f;
            phase_increment = base_frequency / sample_rate;
            active = true;
        }

        inline void note_off()
        {
            active = false;
        }

        /// Update the oscillator’s phase_increment to match a new freq
        inline void set_frequency(float frequency, float sample_rate)
        {
            phase_increment = frequency / sample_rate;
        }

        // change get_sample to no longer shadow the sample_rate param:
        inline float get_sample()
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
