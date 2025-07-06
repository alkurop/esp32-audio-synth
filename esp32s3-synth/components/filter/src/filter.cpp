#include "filter.hpp"
#include <cmath>
#include <algorithm>
#include "filter_table_lp12.hpp"
#include "filter_table_hp12.hpp"
#include "filter_table_bp12.hpp"
#include "filter_table_notch.hpp"
#include "esp_log.h"

#define TAG "Filter"

using namespace protocol;

using namespace sound_module;

Filter::Filter(uint32_t sampleRate, uint8_t init_bpm, uint8_t voiceIndex)
    : sample_rate(sampleRate),
      filterType(FilterType::LP12),
      baseCutoff(MAX_CUTOFF_RAW / 2),       // midpoint default
      baseResonance(MAX_RESONANCE_RAW / 2), // midpoint default
      z1(0.0f),
      z2(0.0f) // midpoint default
{
    resetState();
}

void Filter::resetState()
{
    z1 = 0.0f;
    z2 = 0.0f;
}

float Filter::process(float input)
{
    if (baseCutoff == 0 && baseResonance)
        return input;

    // Quantize and apply modulation (cutting down only)
    // Convert to table indices
    int cutoff_index = static_cast<int>((static_cast<float>(baseCutoff) / MAX_CUTOFF_RAW) * (CUTOFF_TABLE_SIZE - 1));
    int resonance_index = static_cast<int>((static_cast<float>(baseResonance) / MAX_RESONANCE_RAW) * (RESONANCE_TABLE_SIZE - 1));

    cutoff_index = std::clamp(cutoff_index, 0, CUTOFF_TABLE_SIZE - 1);
    resonance_index = std::clamp(resonance_index, 0, RESONANCE_TABLE_SIZE - 1);

    // Update coefficients if filter state changed
    if (cutoff_index != lastCutoffIndex ||
        resonance_index != lastResonanceIndex ||
        filterType != lastFilterType)
    {
        const float (*table)[RESONANCE_TABLE_SIZE][5] = nullptr;

        switch (filterType)
        {
        case FilterType::LP12:
            table = filterTableLP12;
            break;
        case FilterType::HP12:
            table = filterTableHP12;
            break;
        case FilterType::BP12:
            table = filterTableBP12;
            break;
        case FilterType::Notch:
            table = filterTableNotch;
            break;
        default:
            return input;
        }

        const float *coeffs = table[cutoff_index][resonance_index];
        lastB0 = coeffs[0];
        lastB1 = coeffs[1];
        lastB2 = coeffs[2];
        lastA1 = coeffs[3];
        lastA2 = coeffs[4];

        lastCutoffIndex = cutoff_index;
        lastResonanceIndex = resonance_index;
        lastFilterType = filterType;
    }

    // Transposed Direct Form II filter
    float y = lastB0 * input + z1;
    z1 = lastB1 * input + z2 - lastA1 * y;
    z2 = lastB2 * input - lastA2 * y;

    return y;
}
