#pragma once
#include <cstdint>
#include "lfo.hpp"
#include "cached_lfo.hpp"
#include "filter_settings.hpp"

using namespace protocol;

namespace sound_module
{
    /// Generic filter interface: supports dynamic modulation of cutoff and resonance
    class Filter
    {
    public:
        /// Filter types corresponding to protocol::filtTypes

        /// Construct with a sample rate and default settings
        explicit Filter(uint32_t sampleRate, uint8_t init_bpm, uint8_t voiceIndex);

        /// Set the filter type (LP12, HP12, BP12, Notch)
        void setType(FilterType type)
        {
            filterType = type;
            resetState();
        };

        /// Set the base cutoff frequency (Hz) 0 - 127
        void setCutoff(uint8_t cutoffHz) { baseCutoff = cutoffHz; };

        /// Set the base resonance (Q factor)0 - 127
        void setResonance(uint8_t q) { baseResonance = q; };

        /// Process a single sample through the filter, applying any active LFO modulation
        float process(float input, float modulatedCutoff, float modulatedResonance);

    private:
        uint32_t sample_rate;
        FilterType filterType = FilterType::LP12;
        uint8_t baseCutoff = 0;
        uint8_t baseResonance = 0;

        // Internal filter state (poles, etc.)
        float z1 = 0.0f, z2 = 0.0f;
        int lastCutoffIndex = -1;
        int lastResonanceIndex = -1;
        FilterType lastFilterType = FilterType::LP12;

        float lastB0 = 0.0f, lastB1 = 0.0f, lastB2 = 0.0f;
        float lastA1 = 0.0f, lastA2 = 0.0f;
        void resetState();
    };

} // namespace sound_module
