#pragma once
#include <cmath>
#include <cstdint>

namespace sound_module
{
    // Subdivision values expressed in 64th-note units
    enum class LfoSubdivision : uint8_t
    {
        Double = 128,      // 2 whole notes
        Whole = 64,        // whole note
        Half = 32,         // half note
        Quarter = 16,      // quarter note
        Eighth = 8,        // eighth note
        Sixteenth = 4,     // sixteenth note
        DottedQuarter = 24 // dotted quarter note
    };

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

        // Reset phase to start of cycle
        void reset_phase();

        // Get the current LFO output, bipolar range: –depth … +depth
        float get_value();

        uint8_t getDepth();

    private:
        const uint32_t sample_rate;             // samples per second
        uint8_t depth = 0;                      // peak deviation, 0–127
        LfoSubdivision sub = LfoSubdivision::Quarter; // sync subdivision
        uint8_t bpm = 120;                      // beats per minute
        float phase = 0.0f;                     // [0.0, 1.0) cycle phase
    };

} // namespace sound_module
