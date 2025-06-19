#pragma once
#include <cstdint>
#include "oscillator_settings.hpp" // for OscillatorShape, oscShapes, yesNo
#include "envelope.hpp"
#include <math.h>

namespace sound_module
{
    ///
    /// Simple oscillator-based voice
    /// Supports trigger/release, pitch modulation, and waveform settings.
    class Oscillator
    {
    public:
        explicit Oscillator(uint32_t sample_rate, uint16_t initial_bpm);

        /// Trigger the oscillator: set frequency, velocity, and reset phase
        void noteOn(float frequency, uint8_t velocity_in, uint8_t midi_note);

        /// Release the oscillator: mark inactive
        void noteOff();

        /// Update the oscillator’s phase increment to match a new frequency
        void setFrequency(float frequency);

        /// Generate and return one sample at the current phase;
        /// returns zero if inactive
        float getSample();

        /// Configuration setters
        void setShape(protocol::OscillatorShape newShape);
        void setPwm(uint8_t pwm);     // 0–31
        void setBpm(uint16_t bpm);

        /// Configuration getters
        void setVelocity(uint8_t midiVelocity);

        /// Public state for introspection or external use
        uint8_t midi_note = 0;
        float velNorm = 0;
        Envelope envelope; // shared ADSR envelope
        bool isPlaying();
        bool isNoteOn();
        void reset();

    private:
        bool active = false; ///< true if currently playing
        const uint32_t sample_rate;   ///< samples per second
        float phase = 0.0f;           ///< oscillator phase [0,1)
        float phase_increment = 0.0f; ///< increment per sample
        // Oscillator settings
        protocol::OscillatorShape shape = protocol::OscillatorShape::Sine;
        uint8_t pwm = 0;
    };
} // namespace sound_module
