// Generate mixed sample and apply envelope
// voice.cpp
#include "voice.hpp"
#include <cmath>
#include <esp_log.h>
#include <channel_settings.hpp>
#include "cent_pitch_table.hpp"
#include "pan_table.hpp"
#include "esp_attr.h"

using namespace sound_module;
using namespace protocol;

static const char *TAG = "Voice";

IRAM_ATTR Stereo Voice::getSample()
{
    // 0) Master gain smoothing
    float sm_gain = volumeSettings.gain_smoothed.next();
    if (sm_gain <= 1e-6f || activeOscillators.size() == 0)
        return Stereo{0.0f, 0.0f};

    float ampMod = (ampLfoC.getValue() + 127.0f) / 254.0f; // Normalize to 0.0 … 1.0

    float pitchLfoCents = pitchLfoC.getValue(); // This is your LFO value in cents
    float totalCents = pitchSettings.totalTransposeCents + pitchLfoCents;
    float pitchRatio = sound_module::centsToPitchRatio(totalCents);

    // 3) Mix active sounds with pitch, amp, pan
    float mix = 0.0f;

    for (auto iterator = activeOscillators.begin(); iterator != activeOscillators.end();)
    {
        Oscillator *sound = *iterator;

        if (!sound->isPlaying())
        {
            iterator = activeOscillators.erase(iterator);
            // ESP_LOGI(TAG, "Sound erased from voice, new count %d", activeSounds.size());
            continue;
        }

        float modFreq = midi_note_freq[sound->midi_note] * pitchRatio;
        sound->setFrequency(modFreq);

        // float sample = sound->getSample() * sound->velNorm;
        float sample = sound->getSample() * sound->velNorm * ampMod;

        mix += sample;

        ++iterator;
    }
    float cutoffMod = cutoffLfoC.getValue();  
    // float resMod = resonanceLfoC.getValue();  
    // 4) Filter
    mix = filter.process(mix, cutoffMod, 0) * sm_gain;

    // 5) Final gain
    return Stereo{mix, mix};
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
