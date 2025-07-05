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

inline void setSmoothedGain(VolumeSettings &volumeSettings, uint8_t newVolume, uint8_t max, int8_t minDB, int8_t maxDB = 0)
{
    // Clamp to [0..MAX_VOLUME]
    if (newVolume > max)
        newVolume = max;
    volumeSettings.volume = newVolume;

    // 1) Normalize to [0..1]:
    float normalized = static_cast<float>(volumeSettings.volume) / static_cast<float>(max);

    // 2) Map normalized → dB:
    float volume_dB = minDB + normalized * (maxDB - minDB);

    // 3) Convert dB → linear and feed into the smoother:
    float linearGain = std::pow(10.0f, volume_dB * 0.05f);
    // ESP_LOGD(TAG, "Linear gain %f Volume_db %f", linearGain, volume_dB);
    volumeSettings.gain_smoothed.setTarget(linearGain);
}
