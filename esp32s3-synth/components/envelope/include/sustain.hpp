#pragma once
#include "phase.hpp"


class SustainPhase : public EnvelopePhase {
public:
    SustainPhase(float sampleRate);
    void recalculate(uint8_t param, float bpm, float sustainLevel = 0.0f) override;
    void enter(float /*startLevel*/) override;
    float next() override;
    float currentValue() const override;
    bool isFinished() const override;

private:
    float sustainLevel = 0.0f;
};
