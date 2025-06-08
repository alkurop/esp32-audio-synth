#pragma once
#include <cmath>
#include <cstdint>
#include "protocol.hpp"

using namespace protocol;
namespace sound_module
{

    // Generic Low-Frequency Oscillator for modulation of any parameter
    class LFO
    {
    public:
        // Construct the LFO with sample rate, initial BPM, and subdivision
        LFO(uint32_t sampleRate,
            uint8_t initialBpm,
            LfoSubdivision initialSub = LfoSubdivision::Quarter);

        // Set the tempo for sync (beats per minute)
        void setBpm(uint8_t newBpm);

        // Set the sync subdivision
        void setSubdivision(LfoSubdivision s);

        // Set modulation depth (0–127), generic units
        void setDepth(uint8_t newDepth);

        void setWaveform(LfoWaveform wf);

        // Reset phase to start of cycle
        void reset_phase();

        // Get the current LFO output, bipolar range: –depth … +depth
        float get_value();

        uint8_t getDepth();

    private:
        const uint32_t sample_rate;                   // samples per second
        uint8_t depth = 0;                            // peak deviation, 0–127
        LfoSubdivision sub = LfoSubdivision::Quarter; // sync subdivision
        uint8_t bpm = 120;                            // beats per minute
        float phase = 0.0f;                           // [0.0, 1.0) cycle phase
        LfoWaveform waveform;
    };

} // namespace sound_module
