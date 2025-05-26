// envelope.cpp
#include "envelope.hpp"
#include "utils.hpp"

using namespace sound_module;

Envelope::Envelope(float sampleRate)
    : _state(State::Idle),
      _cursor(0),
      _p{0, 0, 7, 0},
      _bpm(120.0f),
      _sr(sampleRate),
      _attSamples(0),
      _decSamples(0),
      _relSamples(0),
      _attRecip(0.0f),
      _decRecip(0.0f),
      _relRecip(0.0f)
{
    setTempo(_bpm);
}

void Envelope::setAttack(uint8_t beats)
{
    _p.attack = (beats > kMaxSteps ? kMaxSteps : beats);
}

void Envelope::setDecay(uint8_t beats)
{
    _p.decay = (beats > kMaxSteps ? kMaxSteps : beats);
}

void Envelope::setSustain(uint8_t level)
{
    _p.sustain = (level > kMaxSteps ? kMaxSteps : level);
}

void Envelope::setRelease(uint8_t beats)
{
    _p.release = (beats > kMaxSteps ? kMaxSteps : beats);
}

uint8_t Envelope::getAttack() const { return _p.attack; }
uint8_t Envelope::getDecay() const { return _p.decay; }
uint8_t Envelope::getSustain() const { return _p.sustain; }
uint8_t Envelope::getRelease() const { return _p.release; }

void Envelope::setTempo(float bpm)
{
    _bpm = bpm;
    float spb = 60.0f / _bpm;

    // Exponential mapping parameters (in beats)
    constexpr float minBeats = 0.125f; // 1/8 beat
    constexpr float maxBeats = 8.0f;   // 8 beats
    float ratio = maxBeats / minBeats;

    // Map steps (0–kMaxSteps) exponentially to beat durations
    float attackBeats = minBeats * powf(ratio, float(_p.attack) / float(kMaxSteps));
    float decayBeats = minBeats * powf(ratio, float(_p.decay) / float(kMaxSteps));
    float releaseBeats = minBeats * powf(ratio, float(_p.release) / float(kMaxSteps));

    // Convert beats to sample counts
    _attSamples = static_cast<uint32_t>(attackBeats * spb * _sr);
    _decSamples = static_cast<uint32_t>(decayBeats * spb * _sr);
    _relSamples = static_cast<uint32_t>(releaseBeats * spb * _sr);

    // Precompute reciprocals for performance
    _attRecip = (_attSamples > 0) ? (1.0f / float(_attSamples)) : 0.0f;
    float sustainNorm = static_cast<float>(_p.sustain) / float(kMaxSteps);
    _decRecip = (_decSamples > 0)
                    ? ((1.0f - sustainNorm) / float(_decSamples))
                    : 0.0f;
    _relRecip = (_relSamples > 0)
                    ? (sustainNorm / float(_relSamples))
                    : 0.0f;
}

void Envelope::note_on()
{
    _state = State::Attack;
    _cursor = 0;
}

void Envelope::note_off()
{
    _state = State::Release;
    _cursor = 0;
}

float Envelope::next()
{
    float out = 0.0f;
    switch (_state)
    {
    case State::Attack:
        out = static_cast<float>(_cursor) * _attRecip;
        if (++_cursor >= _attSamples)
        {
            _state = State::Decay;
            _cursor = 0;
        }
        break;
    case State::Decay:
        out = 1.0f - static_cast<float>(_cursor) * _decRecip;
        if (++_cursor >= _decSamples)
        {
            _state = State::Sustain;
        }
        break;
    case State::Sustain:
        out = static_cast<float>(_p.sustain) / float(kMaxSteps);
        break;
    case State::Release:
        out = (static_cast<float>(_p.sustain) / float(kMaxSteps)) - static_cast<float>(_cursor) * _relRecip;
        if (++_cursor >= _relSamples)
        {
            _state = State::Idle;
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
    return _state == State::Idle;
}
