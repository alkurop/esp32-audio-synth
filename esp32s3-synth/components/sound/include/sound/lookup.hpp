#pragma once
#pragma once
#include <array>
#include <cstddef>

namespace sound_module
{
    template <typename TableT>
    inline float interpolateLookup(float phase, const TableT &table)
    {
        const size_t size = table.size();
        float fidx = phase * size;
        int idx = static_cast<int>(fidx);
        float frac = fidx - idx;

        float a = table[idx % size];
        float b = table[(idx + 1) % size];

        return a + frac * (b - a); // linear interpolation
    }

    inline float poly_blep(float t, float dt)
    {
        if (t < dt)
        {
            float x = t / dt;
            return x + x - x * x - 1.0f;
        }
        else if (t > 1.0f - dt)
        {
            float x = (t - 1.0f) / dt;
            return x * x + x + 1.0f;
        }
        else
        {
            return 0.0f;
        }
    }
}
