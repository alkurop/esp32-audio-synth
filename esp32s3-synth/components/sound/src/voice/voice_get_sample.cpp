// Generate mixed sample and apply envelope
// voice.cpp
#include "voice/voice.hpp"
#include <cmath>
#include "tag.hpp"
#include <esp_log.h>
#include <channel_settings.hpp>

using namespace sound_module;
using namespace protocol;

float Voice::getSample()
{
    // 1) Get the smoothed linear gain each sample:
    float sm_gain = gain_smoothed.next();

    // If effectively silent, skip all per-voice work:
    if (sm_gain <= 1e-6f)
        return 0.0f;

    // 2) Mix all active oscillators:
    float mix = 0.0f;
    int active_count = 0;
    for (auto &s : sounds)
    {
        if (!s.active)
            continue;
        mix += s.get_sample(sample_rate);
        ++active_count;
    }
    if (active_count > 0)
        mix /= static_cast<float>(active_count);

    // 3) Apply the ADSR envelope:
    float env_amp = amp_env.next();

    // 4) Multiply by smoothed gain:
    return mix * env_amp * sm_gain;
}

void Voice::setVolume(uint8_t newVolume)
{
    // Clamp to [0..MAX_VOLUME]
    if (newVolume < 0)
        newVolume = 0;
    else if (newVolume > voice::VOL_MAX)
        newVolume = voice::VOL_MAX;
    volume = newVolume;

    // 1) Normalize to [0..1]:
    float normalized = static_cast<float>(volume) / static_cast<float>(voice::VOL_MAX);

    // 2) Map normalized → dB:
    float volume_dB = voice::VOL_MAX + normalized * (voice::VOL_MAX - voice::MIN_DB);

    // 3) Convert dB → linear and feed into the smoother:
    float linearGain = std::pow(10.0f, volume_dB * 0.05f);
    gain_smoothed.setTarget(linearGain);
}
