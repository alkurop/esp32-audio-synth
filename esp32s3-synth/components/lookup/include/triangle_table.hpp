#pragma once
#include <array>
#include <cmath>
#include "protocol.hpp"

using namespace protocol;

namespace sound_module
{

    const std::array<float, LOOKUP_TABLE_SIZE> triangleTable = []()
    {
        std::array<float, LOOKUP_TABLE_SIZE> table = {};
        for (size_t i = 0; i < LOOKUP_TABLE_SIZE; ++i)
        {
            float phase = static_cast<float>(i) / static_cast<float>(LOOKUP_TABLE_SIZE);
            table[i] = 2.0f * std::fabsf(2.0f * (phase - std::floor(phase + 0.5f))) - 1.0f;
        }
        return table;
    }();
}
