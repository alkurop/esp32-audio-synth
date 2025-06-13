#pragma once
#include "phase.hpp"

class AttackPhase : public EnvelopePhase
{
public:
    explicit AttackPhase(float sampleRate);

    void recalculate(uint8_t param, float bpm, float sustain = 0.0f) override;
    void enter(float /*startLevel*/) override;
    float next() override;
    float currentValue() const override;
    bool isFinished() const override;

private:
    float totalSamples = 1.0f;
    float reciprocal = 1.0f;
    float cursor = 0.0f;
    float currentLevel = 0.0f;
    bool finished = false;
};
