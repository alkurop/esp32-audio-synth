#include "nodes/lfo.hpp"
#include <cmath>

namespace sound_module
{
    // Constructor: initialize sample rate, BPM, subdivision, depth defaults
    LFO::LFO(uint32_t sampleRate,
             uint8_t initialBpm,
             LfoSubdivision initialSub)
        : sample_rate(sampleRate),
          depth(0),
          sub(initialSub),
          bpm(initialBpm),
          phase(0.0f)
    {
    }

    // Set the tempo for sync (beats per minute)
    void LFO::setBpm(uint8_t newBpm)
    {
        bpm = newBpm;
    }

    // Set the sync subdivision
    void LFO::setSubdivision(LfoSubdivision s)
    {
        sub = s;
    }

    // Set modulation depth (0–127), generic units
    void LFO::setDepth(uint8_t newDepth)
    {
        depth = newDepth;
    }

    // Reset phase to start of cycle
    void LFO::reset_phase()
    {
        phase = 0.0f;
    }

    // Get the current LFO output, bipolar range: –depth … +depth
    float LFO::get_value()
    {
        // Compute beats per LFO cycle: subdivision / 64
        float beatsPerCycle = static_cast<float>(static_cast<uint8_t>(sub)) / 64.0f;
        // Convert BPM to Hz: (bpm / 60) beats per second, then / beatsPerCycle for cycles per second
        float rateHz = (static_cast<float>(bpm) / 60.0f) * beatsPerCycle;

        // Generate bipolar sine output in [-1, +1]
        float raw = std::sinf(2.0f * static_cast<float>(M_PI) * phase);

        // Advance phase and wrap
        phase += rateHz / static_cast<float>(sample_rate);
        if (phase >= 1.0f)
            phase -= 1.0f;

        // Scale by depth: output in -depth..+depth
        return raw * static_cast<float>(depth);
    }

    uint8_t LFO::getDepth() { return depth; }

} // namespace sound_module
