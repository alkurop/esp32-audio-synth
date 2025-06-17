#pragma once
#include <cstdint>
#include <math.h>
#include <array>
#include "protocol.hpp"

using namespace protocol;
namespace sound_module

{

    static std::array<float, LOOKUP_TABLE_SIZE> sawTable = []()
    {
        std::array<float, LOOKUP_TABLE_SIZE> table = {};
        for (size_t i = 0; i < LOOKUP_TABLE_SIZE; ++i)
        {
            float phase = static_cast<float>(i) / static_cast<float>(LOOKUP_TABLE_SIZE);
            table[i] = 2.0f * (phase - std::floor(phase + 0.5f)); // Sawtooth waveform
        }
        return table;
    }();

} // namespace sound_module
