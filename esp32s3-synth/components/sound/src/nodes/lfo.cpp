#include "nodes/lfo.hpp"
#include <cmath>

namespace sound_module
{
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
        reset_phase();
    }

    // Set the sync subdivision
    void LFO::setSubdivision(LfoSubdivision s)
    {
        sub = s;
        reset_phase();
    }

    void LFO::setWaveform(LfoWaveform w)
    {
        this->waveform = w;
        reset_phase();
    }

    // Set modulation depth (0–127), generic units
    void LFO::setDepth(uint8_t newDepth) { depth = newDepth; }

    // Reset phase to start of cycle
    void LFO::reset_phase() { phase = 0.0f; }

    // Get the current LFO output, bipolar range: –depth … +depth
    float LFO::get_value()
    {
        // Compute beats per LFO cycle: subdivision / 64
        float beatsPerCycle = static_cast<float>(static_cast<uint8_t>(sub)) / 64.0f;
        // Convert BPM to Hz: (bpm / 60) beats per second, then / beatsPerCycle for cycles per second
        float rateHz = (static_cast<float>(bpm) / 60.0f) * beatsPerCycle;

        float raw = 0.f;
        switch (waveform)
        {
        case LfoWaveform::Sine:
            raw = std::sinf(2.0f * M_PI * phase);
            break;
        case LfoWaveform::Triangle:
            // triangle from –1…+1
            raw = 2.0f * fabsf(2.0f * (phase - floorf(phase + 0.5f))) - 1.0f;
            break;
        case LfoWaveform::Sawtooth:
            // ramp up: –1…+1
            raw = 2.0f * (phase - std::floor(phase)) - 1.0f;
            break;
        case LfoWaveform::Pulse:
            // 50% duty square: +1 first half, –1 second
            raw = (phase < 0.5f) ? 1.0f : -1.0f;
            break;
        default:
            raw = std::sinf(2.0f * M_PI * phase);
        }

        // 2) advance phase
        phase += rateHz / float(sample_rate);
        if (phase >= 1.0f)
            phase -= 1.0f;

        // 3) scale by depth
        return raw * float(depth);
    }

    uint8_t LFO::getDepth() { return depth; }

} // namespace sound_module
