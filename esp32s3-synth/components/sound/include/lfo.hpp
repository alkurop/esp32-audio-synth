#pragma once
#include <cmath>

namespace sound_module
{
    struct LFO
    {
        float phase = 0.0f;
        float rate = 5.0f; // Hz
        float amount = 1.0f;

        float get_value(float sample_rate)
        {
            float value = 0.5f + 0.5f * sinf(2.0f * M_PI * phase); // Range: 0.0 to 1.0
            phase += rate / sample_rate;
            if (phase >= 1.0f)
                phase -= 1.0f;
            return value * amount;
        }
    };
}
