#include "nodes/filter.hpp"
#include <cmath>
#include <algorithm>

using namespace protocol;

using namespace sound_module;

Filter::Filter(uint32_t sampleRate, uint8_t init_bpm)
    : cutoffLfo(sampleRate, init_bpm, LfoSubdivision::Quarter),
      resonanceLfo(sampleRate, init_bpm, LfoSubdivision::Quarter),
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
}

float Filter::process(float input)
{
    if (baseCutoff == 0 && cutoffLfo.getDepth() == 0)
        return input;
    // 1) Modulate cutoff
    float modCut = static_cast<float>(baseCutoff) + cutoffLfo.get_value();
    modCut = std::clamp(modCut, 0.0f, static_cast<float>(MAX_CUTOFF_RAW));
    float nyquist = static_cast<float>(sample_rate) * 0.5f;
    float fc = 20.0f + (modCut / static_cast<float>(MAX_CUTOFF_RAW)) * (nyquist - 20.0f);

    // 2) Modulate resonance
    float modRes = static_cast<float>(baseResonance) + resonanceLfo.get_value();
    modRes = std::clamp(modRes, 0.0f, static_cast<float>(MAX_RESONANCE_RAW));
    float q = 0.1f + (modRes / static_cast<float>(MAX_RESONANCE_RAW)) * (10.0f - 0.1f);

    // 3) Compute biquad coefficients
    float omega = 2.0f * static_cast<float>(M_PI) * fc / static_cast<float>(sample_rate);
    float sn = std::sinf(omega);
    float cs = std::cosf(omega);
    float alpha = sn / (2.0f * q);

    // Biquad SIMD coefficients initialized to safe defaults
    float b0 = 0.0f, b1 = 0.0f, b2 = 0.0f;
    float a0 = 1.0f, a1 = 0.0f, a2 = 0.0f;
    switch (filterType)
    {
    case FilterType::LP12:
        b0 = (1.0f - cs) * 0.5f;
        b1 = 1.0f - cs;
        b2 = b0;
        a0 = 1.0f + alpha;
        a1 = -2.0f * cs;
        a2 = 1.0f - alpha;
        break;
    case FilterType::HP12:
        b0 = (1.0f + cs) * 0.5f;
        b1 = -(1.0f + cs);
        b2 = b0;
        a0 = 1.0f + alpha;
        a1 = -2.0f * cs;
        a2 = 1.0f - alpha;
        break;
    case FilterType::BP12:
        b0 = alpha;
        b1 = 0.0f;
        b2 = -alpha;
        a0 = 1.0f + alpha;
        a1 = -2.0f * cs;
        a2 = 1.0f - alpha;
        break;
    case FilterType::Notch:
        b0 = 1.0f;
        b1 = -2.0f * cs;
        b2 = 1.0f;
        a0 = 1.0f + alpha;
        a1 = -2.0f * cs;
        a2 = 1.0f - alpha;
        break;
    }

    // Normalize coefficients
    b0 /= a0;
    b1 /= a0;
    b2 /= a0;
    a1 /= a0;
    a2 /= a0;

    // 4) Transposed Direct Form II
    float y = b0 * input + z1;
    z1 = b1 * input + z2 - a1 * y;
    z2 = b2 * input - a2 * y;

    return y;
}
