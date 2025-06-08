#pragma once
#include <cstdint>

namespace sound_module
{
    ///
    /// Simple sine-wave voice
    /// Stores MIDI note, base frequency, velocity, and handles phase incrementing.
    class Sound
    {
    public:
        explicit Sound(uint32_t sample_rate);

        /// Trigger a note-on: store MIDI note, frequency, velocity, reset phase
        void note_on(float frequency, float velocity_in);

        /// Trigger a note-off: mark inactive
        void note_off();

        /// Update the oscillator’s phase increment to match a new frequency
        void set_frequency(float frequency);

        /// Generate and return one sample at the current phase;
        /// returns zero if inactive
        float get_sample();

        /// Public state for introspection or external use
        bool active = false;         ///< true if note is on
        uint8_t midi_note = 0;       ///< stored for note-off
        float base_frequency = 0.0f; ///< unmodulated frequency in Hz
        float velocity = 1.0f;       ///< note velocity (gain)

    private:
        const uint32_t sample_rate;   ///< note velocity (gain)
        float phase = 0.0f;           ///< oscillator phase [0,1)
        float phase_increment = 0.0f; ///< increment per sample
    };
} // namespace sound_module
