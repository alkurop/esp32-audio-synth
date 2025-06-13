#pragma once
#include "phase.hpp"

class ReleasePhase : public EnvelopePhase
{
public:
    explicit ReleasePhase(float sampleRate);

    void recalculate(uint8_t param, float bpm, float /*unused*/) override;
    void enter(float startLevel) override;
    float next() override;
    float currentValue() const override;
    bool isFinished() const override;

private:
    float sampleRate;
    float totalSamples = 1.0f;
    float reciprocal = 1.0f;
    float cursor = 0.0f;
    float startLevel = 0.0f;
    float currentLevel = 0.0f;
    bool finished = false;
};
