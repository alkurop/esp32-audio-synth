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

float Voice::getSample()
{
    float lfoValue = pitchLfoC.getValue(); // This is your LFO value in cents
    // 0) Master gain smoothing
    float sm_gain = volumeSettings.gain_smoothed.next();
    if (sm_gain <= 1e-6f || activeSounds.size() == 0)
        return 0.0f;

    // // // 1) Pitch, amplitude, and pan modulation
    // float pitchLfoCents = pitchLfoC.getValue();
    // float ampMod = ampLfoC.getValue() / 127.0f; // Normalize –1.0 … +1.0
    // float ampScale = 1.0f + ampMod;

    // float panMod = panLfoC.getValue() / 127.0f; // Normalize –1.0 … +1.0
    // Stereo pan = getPanGains(panMod);

    // float totalPitchCents = static_cast<float>(totalTransposeCents) + pitchLfoCents;
    // sound->setFrequency(modFreq);

    // float totalPitchCents = static_cast<float>(pitchSettings.pitchRatio);
    // 1) Pitch LFO modulation (in cents → ratio)

    // float pitchLfoCents = pitchLfoC.getValue(); // returns e.g., -127 to +127
    // float pitchLfoRatio = std::pow(2.0f, pitchLfoCents / 1200.0f); // cents → freq ratio

    // float pitchRatio = pitchSettings.pitchRatio * pitchLfoRatio;

    float pitchLfoCents = pitchLfoC.getValue(); // This is your LFO value in cents
    float totalCents = pitchSettings.totalTransposeCents + pitchLfoCents;
    float pitchRatio = sound_module::centsToPitchRatio(totalCents);

    // 3) Mix active sounds with pitch, amp, pan
    float mix = 0.0f;

    for (auto iterator = activeSounds.begin(); iterator != activeSounds.end();)
    {
        Sound *sound = *iterator;

        if (!sound->isPlaying())
        {
            iterator = activeSounds.erase(iterator);
            // ESP_LOGI(TAG, "Sound erased from voice, new count %d", activeSounds.size());
            continue;
        }

        float modFreq = midi_note_freq[sound->midi_note] * pitchRatio;
        sound->setFrequency(modFreq);

        // float sample = sound->getSample() * sound->velNorm;
        float sample = sound->getSample() * sound->velNorm;

        mix += sample;

        ++iterator;
    }

    // 4) Filter
    // mixR = mixL;
    return filter.process(mix) * sm_gain;
    // return mix * sm_gain;

    // 5) Final gain
    // return Stereo{mix * sm_gain, mix * sm_gain};
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
    // ESP_LOGD(TAG, "Linear gain %f Volume_db %f", linearGain, volume_dB);
    volumeSettings.gain_smoothed.setTarget(linearGain);
}
