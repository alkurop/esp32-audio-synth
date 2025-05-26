#pragma once

#include <math.h>

namespace sound_module
{
    inline float clamp(float x, float min_val, float max_val)
    {
        if (x < min_val)
            return min_val;
        if (x > max_val)
            return max_val;
        return x;
    }

    /**
     * Natural logarithm for float values.
     * Wrapper around C's logf for consistency.
     */
    inline float logf(float x)
    {
        return ::logf(x);
    }

    /**
     * Exponential (e^x) for float values.
     * Wrapper around C's expf for consistency.
     */
    inline float expf(float x)
    {
        return ::expf(x);
    }

    /**
     * Power function: x^y = expf(y * logf(x)).
     * Provided for convenience.
     */
    inline float powf(float base, float exponent)
    {
        return expf(exponent * logf(base));
    }
    inline int clamp_midi_note(int note)
    {
        return (note < 0) ? 0 : (note > 127) ? 127
                                             : note;
    }
}
