#pragma once
#include <cstdint>

struct SmoothedGain
{
    float current = 0.0f; // the value we actually output each sample
    float target = 0.0f;  // the “destination” value we want to reach
    float alpha = 0.002f; // smoothing coefficient (controls ramp speed)

    // Call this whenever you want to change the parameter (e.g. volume)
    void setTarget(float newTarget)
    {
        target = newTarget;
    }

    // Call this once per audio sample to step `current` toward `target`.
    float next()
    {
        current += alpha * (target - current);
        return current;
    }
};

struct VolumeSettings
{
    uint8_t volume;
    SmoothedGain gain_smoothed;
};
