// Generate mixed sample and apply envelope
// voice.cpp
#include "voice/voice.hpp"
#include <cmath>
#include <esp_log.h>
#include <channel_settings.hpp>
#include "cent_pitch_table.hpp"
#include "pan_table.hpp"

using namespace sound_module;
using namespace protocol;

static const char *TAG = "Voice";

Stereo Voice::getSample()
{
    // 0) Master gain smoothing
    float sm_gain = volumeSettings.gain_smoothed.next();
    if (sm_gain <= 1e-6f)
        return Stereo{0.0f, 0.0f};

    // // // 1) Pitch, amplitude, and pan modulation
    // float pitchLfoCents = pitchLfoC.getValue();
    // float ampMod = ampLfoC.getValue() / 127.0f; // Normalize –1.0 … +1.0
    // float ampScale = 1.0f + ampMod;

    // float panMod = panLfoC.getValue() / 127.0f; // Normalize –1.0 … +1.0
    // Stereo pan = getPanGains(panMod);

    // float totalPitchCents = static_cast<float>(totalTransposeCents) + pitchLfoCents;
            // sound->setFrequency(modFreq);

    float totalPitchCents = static_cast<float>(totalTransposeCents);
    float pitchRatio = sound_module::centsToPitchRatio(totalPitchCents);


    // 3) Mix active sounds with pitch, amp, pan
    float mixL = 0.0f;
    float mixR = 0.0f;


    for (auto it = activeSounds.begin(); it != activeSounds.end();)
    {
        Sound *sound = *it;

        if (!sound->isPlaying())
        {
            it = activeSounds.erase(it);
            ESP_LOGI(TAG, "Sound erased from voice, new count %d", activeSounds.size());
            continue;
        }

        float modFreq = sound->base_frequency * pitchRatio;
        sound->setFrequency(modFreq);

        float sample = sound->getSample() * sound->velNorm;
        // float sample = sound->getSample() * sound->velNorm * ampScale;

        mixL += sample * 1;
        mixR += sample * 1;

        ++it;
    }

    // 4) Filter
    // mixL = filter.process(mixL);
    // mixR = mixL;
    // mixR = filter.process(mixR);

    // 5) Final gain
    return Stereo{mixL * sm_gain, mixR * sm_gain};
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
