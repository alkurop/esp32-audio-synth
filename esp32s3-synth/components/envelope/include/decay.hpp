#pragma once
#include "phase.hpp"

class DecayPhase : public EnvelopePhase
{
public:
    explicit DecayPhase(float sampleRate);

    void recalculate(uint8_t param, float bpm, float sustainLevel) override;
    void enter(float startLevel) override;
    float next() override;
    float currentValue() const override;
    bool isFinished() const override;

private:
    float sampleRate;
    float totalSamples = 1.0f;
    float reciprocal = 1.0f;
    float cursor = 0.0f;
    float currentLevel = 1.0f;

    float startLevel = 1.0f;
    float targetLevel = 0.6f;
    bool finished = false;
};
