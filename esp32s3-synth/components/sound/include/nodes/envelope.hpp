// envelope.hpp (updated)
#pragma once
#include <cstdint>

namespace sound_module
{

    /**
     * ADSR envelope: parameters are integer steps (0–7).
     * Attack, Decay, Release are in beats (0–7).
     * Sustain is amplitude level in steps (0–7), where 7 = full volume.
     */
    class Envelope
    {
    public:
        static constexpr uint8_t kMaxSteps = 7;

        struct Params
        {
            uint8_t attack;  // Attack time in beats (0–7)
            uint8_t decay;   // Decay time in beats (0–7)
            uint8_t sustain; // Sustain level in steps (0–7)
            uint8_t release; // Release time in beats (0–7)
        };

        // Construct with fixed sample rate (Hz)
        explicit Envelope(float sampleRate);

        // Setters / getters for each parameter (clamped to [0, kMaxSteps])
        void setAttack(uint8_t beats);
        void setDecay(uint8_t beats);
        void setSustain(uint8_t level);
        void setRelease(uint8_t beats);

        uint8_t getAttack() const;
        uint8_t getDecay() const;
        uint8_t getSustain() const;
        uint8_t getRelease() const;

        // Call when BPM changes; must precede note_on()
        void setTempo(float bpm);

        // Trigger envelope phases
        void noteOn();
        void noteOff();

        // Advance one sample; returns amplitude [0.0f–1.0f]
        float next();

        // True when envelope has completed the release phase
        bool is_idle() const;

    private:
        enum class State
        {
            Idle,
            Attack,
            Decay,
            Sustain,
            Release
        } state{State::Idle};
        uint32_t cursor{0};

        Params params{0, 0, 7, 0};
        float bpm{120.0f};
        const float sampleRate; // fixed sample rate

        // Sample counts for phases
        uint32_t attackSamples{0};
        uint32_t decaySamples{0};
        uint32_t releaseSamples{0};

        // Reciprocals for fast multiplications
        float attackReciprocals{0.0f};
        float decayReciprocals{0.0f};
        float releaseReciprocals{0.0f};
    };

} // namespace sound_module
