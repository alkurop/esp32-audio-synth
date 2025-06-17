#pragma once
#include <cstdint>
#include <array>
#include <cstddef>
#include <complex>
#include <algorithm>

namespace sound_module
{
    constexpr size_t CENT_PITCH_TABLE_SIZE = 241; // For –120 to +120 cents
    constexpr int CENT_OFFSET = 120;              // To shift index from –120...+120 → 0...240

    inline const std::array<float, CENT_PITCH_TABLE_SIZE> centPitchTable = []()
    {
        std::array<float, CENT_PITCH_TABLE_SIZE> table = {};
        for (size_t i = 0; i < CENT_PITCH_TABLE_SIZE; ++i)
        {
            int cents = static_cast<int>(i) - CENT_OFFSET;
            table[i] = std::pow(2.0f, cents / 1200.0f);
        }
        return table;
    }();
    /// Convert cents to pitch ratio using lookup table
    inline float centsToPitchRatio(float cents)
    {
        int index = static_cast<int>(std::round(cents)) + CENT_OFFSET;
        index = std::clamp(index, 0, static_cast<int>(CENT_PITCH_TABLE_SIZE - 1));
        return centPitchTable[index];
    }
}
