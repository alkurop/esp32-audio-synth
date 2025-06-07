// envelope.cpp
#include "nodes/envelope.hpp"
#include "utils.hpp"
#include "protocol.hpp"

using namespace sound_module;
using namespace protocol;

inline auto clamp_to_envelope_max(int x) noexcept { return (x > envelope::MAX ? envelope::MAX : x); }

Envelope::Envelope(float sampleRate, uint16_t initialBpm)
    : state(State::Idle),
      cursor(0),
      params{0, 0, 0, 0},
      sampleRate(sampleRate),
      attackSamples(0),
      decaySamples(0),
      releaseSamples(0),
      attackReciprocals(0.0f),
      decayReciprocals(0.0f),
      releaseReciprocals(0.0f)
{
    setBpm(initialBpm);
}

void Envelope::setAttack(uint8_t value)
{
    params.attack = clamp_to_envelope_max(value);
    recalculate();
}
void Envelope::setDecay(uint8_t value)
{
    params.decay = clamp_to_envelope_max(value);
    recalculate();
}
void Envelope::setSustain(uint8_t value)
{
    params.sustain = clamp_to_envelope_max(value);
    recalculate();
}
void Envelope::setRelease(uint8_t value)
{
    params.release = clamp_to_envelope_max(value);
    recalculate();
}

void Envelope::setBpm(uint16_t bpm)
{
    this->bpm = bpm;
    recalculate();
}
void Envelope::recalculate()
{
    // (1) Compute seconds‐per‐beat:
    float spb = 60.0f / bpm;
    //      ↑–––––––––––––––––––↑
    //      If bpm = 120, spb = 0.5 seconds per beat.

    // (2) Decide what “value” (in beats) each ADSR stage should last.
    //
    //     You map the parameter integer (0 … envelope::MAX) to a
    //     number of beats between minvalue (0.125 beats) and maxvalue (8.0 beats).
    constexpr float minvalue = 0.125f; // 1/8 of a beat
    constexpr float maxvalue = 8.0f;   // 8 beats
    float ratio = maxvalue / minvalue; // = 64.0

    //    For each ADSR parameter (attack, decay, release), you compute:
    float attackvalue = minvalue * powf(ratio, float(params.attack) / float(envelope::MAX));
    float decayvalue = minvalue * powf(ratio, float(params.decay) / float(envelope::MAX));
    float releasevalue = minvalue * powf(ratio, float(params.release) / float(envelope::MAX));

    //    Example: if params.attack = 0,
    //      attackvalue = 0.125 beats
    //    if params.attack = envelope::MAX,
    //      attackvalue = 8.0 beats
    //
    //    Anything in between is on an exponential curve from 1/8 beat → 8 beats.

    // (3) Convert “beats” to actual sample counts:
    attackSamples = uint32_t(attackvalue * spb * sampleRate);
    decaySamples = uint32_t(decayvalue * spb * sampleRate);
    releaseSamples = uint32_t(releasevalue * spb * sampleRate);

    //    For example, if sampleRate = 48 kHz, bpm = 120 → spb = 0.5 s/beat:
    //      attackvalue = 1 beat  → attackSamples = 1 × 0.5 s × 48000 = 24,000 samples.
    //      releasevalue = 4 beats → releaseSamples = 4 × 0.5 s × 48000 = 96,000 samples.
    //
    // (4) Precompute reciprocals for performance in next():
    attackReciprocals = (attackSamples > 0 ? 1.0f / float(attackSamples) : 0.0f);
    float sustainNorm = float(params.sustain) / float(envelope::MAX);
    decayReciprocals = (decaySamples > 0 ? ((1.0f - sustainNorm) / float(decaySamples)) : 0.0f);
    releaseReciprocals = (releaseSamples > 0 ? (sustainNorm / float(releaseSamples)) : 0.0f);
}

void Envelope::gateOn()
{
    state = State::Attack;
    cursor = 0;
}

void Envelope::gateOff()
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
        out = static_cast<float>(params.sustain) / float(envelope::MAX);
        break;
    case State::Release:
        out = (static_cast<float>(params.sustain) / float(envelope::MAX)) - static_cast<float>(cursor) * releaseReciprocals;
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
