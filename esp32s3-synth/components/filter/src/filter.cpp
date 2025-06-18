#include "filter.hpp"
#include <cmath>
#include <algorithm>
#include "filter_table_lp12.hpp"
#include "filter_table_hp12.hpp"
#include "filter_table_bp12.hpp"
#include "filter_table_notch.hpp"

using namespace protocol;

using namespace sound_module;

Filter::Filter(uint32_t sampleRate, uint8_t init_bpm, uint8_t voiceIndex)
    : cutoffLfo(sampleRate, init_bpm, LfoSubdivision::Quarter),
      resonanceLfo(sampleRate, init_bpm, LfoSubdivision::Quarter),
      cutoffLfoC(cutoffLfo, 8, voiceIndex),
      resonanceLfoC(resonanceLfo, 8, voiceIndex + 1),
      sample_rate(sampleRate),
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
    cutoffLfo.resetPhase();
    resonanceLfo.resetPhase();
}
float Filter::process(float input)
{
    if (baseCutoff == 0 && cutoffLfo.getDepth() == 0)
        return input;

    // 1. Modulate cutoff
    float modCut = static_cast<float>(baseCutoff) + cutoffLfoC.getValue();
    modCut = std::clamp(modCut, 0.0f, static_cast<float>(MAX_CUTOFF_RAW));
    int cutoff_index = static_cast<int>((modCut / MAX_CUTOFF_RAW) * (CUTOFF_TABLE_SIZE - 1));

    // 2. Modulate resonance
    float modRes = static_cast<float>(baseResonance) + resonanceLfoC.getValue();
    modRes = std::clamp(modRes, 0.0f, static_cast<float>(MAX_RESONANCE_RAW));
    int resonance_index = static_cast<int>((modRes / MAX_RESONANCE_RAW) * (RESONANCE_TABLE_SIZE - 1));

    const float* coeffs = nullptr;

    // 3. Select coefficients from table based on filter type
    switch (filterType)
    {
    case FilterType::LP12:
        coeffs = filterTableLP12[cutoff_index][resonance_index];
        break;
    case FilterType::HP12:
        coeffs = filterTableHP12[cutoff_index][resonance_index];
        break;
    case FilterType::BP12:
        coeffs = filterTableBP12[cutoff_index][resonance_index];
        break;
    case FilterType::Notch:
        coeffs = filterTableNotch[cutoff_index][resonance_index];
        break;
    default:
        return input; // Unknown type or not yet supported
    }

    // 4. Apply coefficients using Transposed Direct Form II
    float b0 = coeffs[0];
    float b1 = coeffs[1];
    float b2 = coeffs[2];
    float a1 = coeffs[3];
    float a2 = coeffs[4];

    float y = b0 * input + z1;
    z1 = b1 * input + z2 - a1 * y;
    z2 = b2 * input - a2 * y;

    return y;
}
