// envelope.cpp
#include "nodes/envelope.hpp"
#include "utils.hpp"

using namespace sound_module;

Envelope::Envelope(float sampleRate)
    : state(State::Idle),
      cursor(0),
      params{0, 0, 7, 0},
      bpm(120.0f),
      sampleRate(sampleRate),
      attackSamples(0),
      decaySamples(0),
      releaseSamples(0),
      attackReciprocals(0.0f),
      decayReciprocals(0.0f),
      releaseReciprocals(0.0f)
{
    setTempo(bpm);
}

void Envelope::setAttack(uint8_t beats)
{
    params.attack = (beats > kMaxSteps ? kMaxSteps : beats);
}

void Envelope::setDecay(uint8_t beats)
{
    params.decay = (beats > kMaxSteps ? kMaxSteps : beats);
}

void Envelope::setSustain(uint8_t level)
{
    params.sustain = (level > kMaxSteps ? kMaxSteps : level);
}

void Envelope::setRelease(uint8_t beats)
{
    params.release = (beats > kMaxSteps ? kMaxSteps : beats);
}

uint8_t Envelope::getAttack() const { return params.attack; }
uint8_t Envelope::getDecay() const { return params.decay; }
uint8_t Envelope::getSustain() const { return params.sustain; }
uint8_t Envelope::getRelease() const { return params.release; }

void Envelope::setTempo(float bpm)
{
    bpm = bpm;
    float spb = 60.0f / bpm;

    // Exponential mapping parameters (in beats)
    constexpr float minBeats = 0.125f; // 1/8 beat
    constexpr float maxBeats = 8.0f;   // 8 beats
    float ratio = maxBeats / minBeats;

    // Map steps (0–kMaxSteps) exponentially to beat durations
    float attackBeats = minBeats * powf(ratio, float(params.attack) / float(kMaxSteps));
    float decayBeats = minBeats * powf(ratio, float(params.decay) / float(kMaxSteps));
    float releaseBeats = minBeats * powf(ratio, float(params.release) / float(kMaxSteps));

    // Convert beats to sample counts
    attackSamples = static_cast<uint32_t>(attackBeats * spb * sampleRate);
    decaySamples = static_cast<uint32_t>(decayBeats * spb * sampleRate);
    releaseSamples = static_cast<uint32_t>(releaseBeats * spb * sampleRate);

    // Precompute reciprocals for performance
    attackReciprocals = (attackSamples > 0) ? (1.0f / float(attackSamples)) : 0.0f;
    float sustainNorm = static_cast<float>(params.sustain) / float(kMaxSteps);
    decayReciprocals = (decaySamples > 0)
                    ? ((1.0f - sustainNorm) / float(decaySamples))
                    : 0.0f;
    releaseReciprocals = (releaseSamples > 0)
                    ? (sustainNorm / float(releaseSamples))
                    : 0.0f;
}

void Envelope::noteOn()
{
    state = State::Attack;
    cursor = 0;
}

void Envelope::noteOff()
{
    state = State::Release;
    cursor = 0;
}

float Envelope::next()
{
    float out = 0.0f;
    switch (state)
    {
    case State::Attack:
        out = static_cast<float>(cursor) * attackReciprocals;
        if (++cursor >= attackSamples)
        {
            state = State::Decay;
            cursor = 0;
        }
        break;
    case State::Decay:
        out = 1.0f - static_cast<float>(cursor) * decayReciprocals;
        if (++cursor >= decaySamples)
        {
            state = State::Sustain;
        }
        break;
    case State::Sustain:
        out = static_cast<float>(params.sustain) / float(kMaxSteps);
        break;
    case State::Release:
        out = (static_cast<float>(params.sustain) / float(kMaxSteps)) - static_cast<float>(cursor) * releaseReciprocals;
        if (++cursor >= releaseSamples)
        {
            state = State::Idle;
            out = 0.0f;
        }
        break;
    case State::Idle:
    default:
        out = 0.0f;
        break;
    }
    return out;
}

bool Envelope::is_idle() const
{
    return state == State::Idle;
}
