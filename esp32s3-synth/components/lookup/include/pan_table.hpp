#pragma once
#include <cstdint>
#include <array>
#include <cstddef>
#include <complex>
#include <algorithm>
#include "stereo.hpp"

namespace sound_module
{
    constexpr size_t PAN_TABLE_SIZE = 129; // from -1.0 to +1.0 in 128 steps

    const std::array<Stereo, PAN_TABLE_SIZE> panTable = []()
    {
        std::array<Stereo, PAN_TABLE_SIZE> table = {};
        for (size_t i = 0; i < PAN_TABLE_SIZE; ++i)
        {
            float pan = (static_cast<float>(i) / (PAN_TABLE_SIZE - 1)) * 2.0f - 1.0f;
            float angle = (pan + 1.0f) * (static_cast<float>(M_PI) / 4.0f); // 0 to π/2
            table[i] = Stereo{std::cosf(angle), std::sinf(angle)};
        }
        return table;
    }();

    inline Stereo getPanGains(float pan)
    {
        int index = static_cast<int>((pan + 1.0f) * 64); // Map -1.0…+1.0 → 0…128
        index = std::clamp(index, 0, static_cast<int>(PAN_TABLE_SIZE - 1));
        return panTable[index];
    }

} // namespace sound_module
