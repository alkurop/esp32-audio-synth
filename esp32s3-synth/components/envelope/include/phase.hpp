#pragma once
#include <cstdint>
#include "protocol.hpp"
#include <cmath>

using namespace protocol;

class EnvelopePhase
{
public:
    explicit EnvelopePhase(float sampleRate) : sampleRate(sampleRate) {}

    virtual void recalculate(uint8_t param, float bpm, float sustainLevel = 0.0f) = 0;
    virtual void enter(float startLevel) = 0;
    virtual float next() = 0;
    virtual float currentValue() const = 0;
    virtual bool isFinished() const = 0;

protected:
    const float sampleRate;

    float calcBeats(uint8_t param, float bpm) const
    {
        float norm = float(param) / float(envelope::MAX);
        float curved = 1.0f - norm; // More precision on short durations
        return envelope::MIN_BEAT_LENGTH * std::pow(envelope::MAX_BEAT_LENGTH / envelope::MIN_BEAT_LENGTH, curved);
    }

    float beatsToSamples(float beats, float bpm) const
    {
        return beats * (60.0f / bpm) * sampleRate;
    }
};
