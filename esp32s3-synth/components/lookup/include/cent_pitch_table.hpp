#pragma once
#include <cstdint>
#include <array>
#include <cstddef>
#include <complex>
#include <algorithm>

namespace sound_module
{
    constexpr int CENT_MIN = -2400;
    constexpr int CENT_MAX = 2400;
    constexpr size_t CENT_PITCH_TABLE_SIZE = CENT_MAX - CENT_MIN + 1;
    constexpr int CENT_OFFSET = -CENT_MIN;

    const std::array<float, CENT_PITCH_TABLE_SIZE> centPitchTable = []()
    {
        std::array<float, CENT_PITCH_TABLE_SIZE> table = {};
        for (size_t i = 0; i < CENT_PITCH_TABLE_SIZE; ++i)
        {
            int cents = static_cast<int>(i) + CENT_MIN;
            table[i] = std::pow(2.0f, cents / 1200.0f);
        }
        return table;
    }();
    inline float centsToPitchRatio(float cents)
    {
        int index = static_cast<int>(std::round(cents)) + CENT_OFFSET;
        index = std::clamp(index, 0, static_cast<int>(CENT_PITCH_TABLE_SIZE - 1));
        return centPitchTable[index];
    }
}
