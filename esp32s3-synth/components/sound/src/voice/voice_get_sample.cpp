// Generate mixed sample and apply envelope
// voice.cpp
#include "voice/voice.hpp"
#include <cmath>
#include <esp_log.h>
#include <channel_settings.hpp>

using namespace sound_module;
using namespace protocol;

static const char *TAG = "Voice";

float Voice::getSample()
{
    // 0) Master gain smoothing
    float sm_gain = volumeSettings.gain_smoothed.next();
    if (sm_gain <= 1e-6f)
        return 0.0f;
 
    float mix = 0.0f;
    for (auto &sound : sounds)
    {
        if (!sound.isPlaying())
            continue;

        sound.set_frequency(sound.base_frequency); // with vibrato etc.
        mix += sound.get_sample() * sound.velNorm;
    }
    return mix * sm_gain;
}

void Voice::setVolume(uint8_t newVolume)
{
    // Clamp to [0..MAX_VOLUME]
    if (newVolume > voice::VOL_MAX)
        newVolume = voice::VOL_MAX;
    volumeSettings.volume = newVolume;

    // 1) Normalize to [0..1]:
    float normalized = static_cast<float>(volumeSettings.volume) / static_cast<float>(voice::VOL_MAX);

    // 2) Map normalized → dB:
    float volume_dB = voice::MIN_DB + normalized * (0.0f - voice::MIN_DB);

    // 3) Convert dB → linear and feed into the smoother:
    float linearGain = std::pow(10.0f, volume_dB * 0.05f);
    ESP_LOGD(TAG, "Linear gain %f Volume_db %f", linearGain, volume_dB);
    volumeSettings.gain_smoothed.setTarget(linearGain);
}
