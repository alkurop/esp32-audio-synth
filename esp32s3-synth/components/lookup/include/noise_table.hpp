#pragma once
#include <cstdint>
#include <array>
#include "protocol.hpp"
using namespace protocol;

namespace sound_module
{
    // Deterministic “noise” function per index — no globals, fully reproducible
    inline float computeNoise(int i)
    {
        // 32-bit integer hashing (bit‐mixing) → [0,1] → [-1,1]
        uint32_t x = static_cast<uint32_t>(i) * 2654435761u;
        x ^= x >> 16;
        x *= 2246822519u;
        x ^= x >> 13;
        return (static_cast<float>(x) / 4294967295.0f) * 2.0f - 1.0f;
    };

    // Static noise table initialized once at program startup,
    // exactly the same pattern as your sineTable
    const std::array<float, LOOKUP_TABLE_SIZE> noiseTable = []()
    {
        std::array<float, LOOKUP_TABLE_SIZE> table = {};
        for (int i = 0; i < LOOKUP_TABLE_SIZE; ++i)
        {
            table[i] = computeNoise(i);
        }
        return table;
    }();

} // namespace sound_module
