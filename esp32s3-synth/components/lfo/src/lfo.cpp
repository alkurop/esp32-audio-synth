#include "lfo.hpp"
#include <cmath>
#include "protocol.hpp"
#include "sine_table.hpp"
#include "saw_tooth_table.hpp"
#include "triangle_table.hpp"
#include "square_table.hpp"
#include "lookup.hpp"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_attr.h"

static const char *TAG = "Lfo";
static constexpr float MICROSECONDS_TO_SECONDS = 1.0f / 1'000'000.0f;

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
void LFO::setDepth(uint8_t newDepth)
{
    // ESP_LOGI(TAG, "Set depth %d", newDepth);

    depth = newDepth;
}

// Reset phase to start of cycle
void LFO::resetPhase()
{
    phase = 0.0f;
    cyclesPerSecond = (static_cast<float>(bpm) / 60.0f) / beatsPerCycleMap[static_cast<int>(sub)];
}

// Get the current LFO output, bipolar range: –depth … +depth
IRAM_ATTR float LFO::getValue()
{
    if (depth == 0)
        return 0.0f;

    float raw = 0.f;
    switch (waveform)
    {
    case LfoWaveform::Sine:
        raw = interpolateLookup(phase, sineTable);
        break;
    case LfoWaveform::Triangle:
        raw = interpolateLookup(phase, triangleTable);
        break;
    case LfoWaveform::Sawtooth:
        raw = interpolateLookup(phase, sawTable);
        break;
    case LfoWaveform::Pulse:
        raw = (phase < 0.5f) ? 1.0f : -1.0f;
        break;
    default:
        raw = 0.0f;
    }

    return raw * float(depth);
}

uint8_t LFO::getDepth() { return depth; }

void LFO::advancePhaseMicroseconds(uint32_t elapsed_us)
{
    if (depth == 0)
        return;
    // Avoid recalculating BPM and beatsPerCycle each time
    phase += cyclesPerSecond * elapsed_us * MICROSECONDS_TO_SECONDS;

    // Faster wraparound using subtraction instead of floorf
    if (phase >= 1.0f)
        phase -= static_cast<int>(phase); // remove integer part
}
