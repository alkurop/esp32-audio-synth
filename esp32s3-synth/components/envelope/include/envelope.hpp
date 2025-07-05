#pragma once
#include "phase.hpp"
#include "attack.hpp"
#include "decay.hpp"
#include "sustain.hpp"
#include "release.hpp"

class Envelope
{
public:
    struct Params
    {
        uint8_t attack;
        uint8_t decay;
        uint8_t sustain;
        uint8_t release;
    };

    explicit Envelope(float sampleRate, uint16_t initialBpm);

    void setAttack(uint8_t value);
    void setDecay(uint8_t value);
    void setSustain(uint8_t value);
    void setRelease(uint8_t value);
    void setBpm(uint16_t bpm);

    void gateOn();
    void gateOff();
    void setToIdle();

    float next();
    bool is_idle() const;

private:
    Params params{0, 0, 0, 0};
    float sustainLevel = 0.0f;
    float sampleRate;
    uint16_t bpm;

    // ADSR phases
    AttackPhase attack;
    DecayPhase decay;
    SustainPhase sustain;
    ReleasePhase release;

    EnvelopePhase *active = nullptr;

    void recalculate(); // update all phases based on current params
};
