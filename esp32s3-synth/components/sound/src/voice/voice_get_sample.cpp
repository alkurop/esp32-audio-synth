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

    // // 1) Tremolo: bipolar LFO output in –depth…+depth → map to [0…1]
    // float ampBipolar = amp_lfo.get_value(); // –depth…+depth
    // float tremDepth = static_cast<float>(amp_lfo.getDepth());
    // float tremUnipolar = 0.5f * (ampBipolar / tremDepth + 1.0f);
    // float tremGain = tremUnipolar; // now in [0…1]

    // // 2) Vibrato: bipolar LFO in cents
    // float vibCents = pitch_lfo.get_value();              // –depth…+depth cents
    // float pitchMul = std::pow(2.0f, vibCents / 1200.0f); // cents → freq multiplier

    // 3) Mix all active voices with per-voice vibrato
    float mix = 0.0f;
    int activeCount = 0;
    for (auto &sound : sounds)
    {
        if (!sound.active)
            continue;

        float modFreq = sound.base_frequency; // or with vibrato mod
        sound.set_frequency(modFreq);

        // Pre-attenuate to handle polyphony headroom (e.g. 5 voices max)
        mix += sound.get_sample() * 0.2f; // or (1.0f / max_polyphony)
        ++activeCount;
    }

    if (activeCount > 0)
        mix /= static_cast<float>(activeCount); // ✅ average

    // 4) ADSR envelope
    float envAmp = envelope.next();

    // 5) Final output: mix → envelope → master gain → tremolo
    return mix * sm_gain;
    // return mix * envAmp * sm_gain * tremGain;
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
