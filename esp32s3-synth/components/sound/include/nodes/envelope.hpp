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
        struct Params
        {
            uint8_t attack;  // Attack time in beats (0–7)
            uint8_t decay;   // Decay time in beats (0–7)
            uint8_t sustain; // Sustain level in steps (0–7)
            uint8_t release; // Release time in beats (0–7)
        };

        // Construct with fixed sample rate (Hz)
        explicit Envelope(float sampleRate, uint16_t initialBpm);

        // Setters / getters for each parameter (clamped to [0, kMaxSteps])
        void setSustain(uint8_t beats);
        void setSustain(uint8_t beats);
        void setSustain(uint8_t level);
        void setRelease(uint8_t beats);

 

        // Call when BPM changes; must precede note_on()
        void setBpm(uint16_t bpm);

        // Trigger envelope phases
        void gateOn();
        void gateOff();

       
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

        Params params{0, 0, 0, 0};
        const float sampleRate; // fixed sample rate
        uint16_t bpm;
        // Sample counts for phases
        uint32_t attackSamples{0};
        uint32_t decaySamples{0};
        uint32_t releaseSamples{0};

        // Reciprocals for fast multiplications
        float attackReciprocals{0.0f};
        float decayReciprocals{0.0f};
        float releaseReciprocals{0.0f};
        void recalculate();
    };

} // namespace sound_module
