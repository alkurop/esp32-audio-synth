#pragma once
#include <cstdint>
#include <math.h>
#include <array>
#include "protocol.hpp"

namespace sound_module

{

    inline float computeSine(int i)
    {
        return std::sinf(2.0f * static_cast<float>(M_PI) * i / protocol::TABLE_SIZE);
    };

    // Static sine table initialized once at program startup
    static std::array<float, protocol::TABLE_SIZE> sineTable = []()
    {
        std::array<float, protocol::TABLE_SIZE> table = {};
        for (int i = 0; i < protocol::TABLE_SIZE; ++i)
        {
            table[i] = computeSine(i);
        }
        return table;
    }();

} // namespace sound_module
