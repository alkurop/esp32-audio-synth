#pragma once
#include <cstdint>
#include "oscillator_settings.hpp" // for OscillatorShape, oscShapes, yesNo
#include "../nodes/envelope.hpp"
#include <math.h>

namespace sound_module
{
    ///
    /// Simple oscillator-based voice
    /// Supports trigger/release, pitch modulation, and waveform settings.
    class Sound
    {
    public:
        explicit Sound(uint32_t sample_rate, uint16_t initial_bpm);

        /// Trigger the oscillator: set frequency, velocity, and reset phase
        void trigger(float frequency, uint8_t velocity_in, uint8_t midi_note);

        /// Release the oscillator: mark inactive
        void release();

        /// Update the oscillator’s phase increment to match a new frequency
        void set_frequency(float frequency);

        /// Generate and return one sample at the current phase;
        /// returns zero if inactive
        float get_sample();

        /// Configuration setters
        void set_shape(protocol::OscillatorShape newShape);
        void set_morph(uint8_t morph); // 0–31
        void set_pwm(uint8_t pwm);     // 0–31
        void set_sync(bool sync_on);
        void setBpm(uint16_t bpm);

        /// Configuration getters
        protocol::OscillatorShape get_shape() const;
        uint8_t get_morph() const;
        uint8_t get_pwm() const;
        void setVelocity(uint8_t midiVelocity);

        /// Public state for introspection or external use
        float base_frequency = 0.0f; ///< unmodulated frequency in Hz
        uint8_t midi_note = 0;
        float velNorm = 0;
        Envelope envelope; // shared ADSR envelope
        bool isActive();

    private:
        bool active = false; ///< true if currently playing
        bool sync = false;
        const uint32_t sample_rate;   ///< samples per second
        float phase = 0.0f;           ///< oscillator phase [0,1)
        float phase_increment = 0.0f; ///< increment per sample
        // Oscillator settings
        protocol::OscillatorShape shape = protocol::OscillatorShape::Sine;
        uint8_t morph = 0;
        uint8_t pwm = 0;
    };

    inline float fast_sinf(float x)
    {
        const float B = 4.0f / M_PI;
        const float C = -4.0f / (M_PI * M_PI);
        const float P = 0.225f;

        float y = B * x + C * x * std::fabsf(x);
        return P * (y * std::fabsf(y) - y) + y;
    }
} // namespace sound_module
