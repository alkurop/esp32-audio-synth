#include "lfo.hpp"
#include <cmath>
#include "protocol.hpp"
#include "sine_table.hpp"
#include "saw_tooth_table.hpp"
#include "triangle_table.hpp"
#include "square_table.hpp"
#include "lookup.hpp"
using namespace sound_module;
// Constructor: initialize sample rate, BPM, subdivision, depth defaults
LFO::LFO(uint32_t sampleRate, uint8_t initialBpm, LfoSubdivision initialSub)
    : sample_rate(sampleRate),
      depth(0), sub(initialSub),
      bpm(initialBpm), phase(0.0f),
      waveform(LfoWaveform::Sine)
{
}

// Set the tempo for sync (beats per minute)
void LFO::setBpm(uint8_t newBpm)
{
    bpm = newBpm;
    resetPhase();
}

// Set the sync subdivision
void LFO::setSubdivision(LfoSubdivision s)
{
    sub = s;
    resetPhase();
}

void LFO::setWaveform(LfoWaveform w)
{
    this->waveform = w;
    resetPhase();
}

// Set modulation depth (0–127), generic units
void LFO::setDepth(uint8_t newDepth) { depth = newDepth; }

// Reset phase to start of cycle
void LFO::resetPhase() { phase = 0.0f; }

// Get the current LFO output, bipolar range: –depth … +depth
float LFO::getValue(uint16_t ticks)
{

    if (depth == 0)
        return 0.0f;
    advancePhase(ticks); // scale phase increment correctly

    float raw = 0.f;
    switch (waveform)
    {
    case LfoWaveform::Sine:
        raw = interpolateLookup(phase, sineTable);
        break;
    case LfoWaveform::Triangle:
        // triangle from –1…+1
        raw = interpolateLookup(phase, triangleTable);
        break;
    case LfoWaveform::Sawtooth:
        // ramp up: –1…+1
        raw = interpolateLookup(phase, sawTable);
        break;
    case LfoWaveform::Pulse:
        // 50% duty square: +1 first half, –1 second
        raw = (phase < 0.5f) ? 1.0f : -1.0f;
        break;
    default:
        raw = 0.0f;
    }

    return raw * float(depth);
}

uint8_t LFO::getDepth() { return depth; }

void LFO::advancePhase(uint16_t ticks)
{
    float beatsPerCycle = static_cast<float>(static_cast<uint8_t>(sub)) / 64.0f;
    float rateHz = (static_cast<float>(bpm) / 60.0f) * beatsPerCycle;

    phase += ticks * (rateHz / float(sample_rate));
    if (phase >= 1.0f)
        phase -= floorf(phase); // wraparound
}
