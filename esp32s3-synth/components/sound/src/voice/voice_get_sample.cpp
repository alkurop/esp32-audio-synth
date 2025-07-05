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

#define TAG = "Voice";

Stereo Voice::getSample()
{
    // 0) Always clean up first
    garbageCollect();

    // 1) If nothing left, bail out immediately
    if (activeOscillators.empty())
        return {0.0f, 0.0f};

    // 2) Compute modulators
    float sm_gain = volumeSettings.gain_smoothed.next();
    float ampRaw = (ampLfoC.getValue() + 127.0f) / 254.0f;
    ampLfoSmoothed = AMP_ALPHA * ampRaw + (1.0f - AMP_ALPHA) * ampLfoSmoothed;
    float pitchRaw = pitchLfoC.getValue();
    float pitchOffset = pitchRaw * (pitchLfoDepth / 127.0f);
    float totalCents = pitchSettings.totalTransposeCents + pitchOffset;
    float pitchRatio = sound_module::centsToPitchRatio(totalCents);

    // 3) Mix & step every remaining voice (all are playing)
    float mix = 0.0f;
    for (auto *s : activeOscillators)
    {
        s->setFrequency(midi_note_freq[s->midi_note] * pitchRatio);
        mix += s->getSample() * s->velNorm * ampLfoSmoothed;
    }

    // 4) Filter + master gain
    mix = filter.process(mix) * sm_gain;

    return {mix, mix};
}

void Voice::setVolume(uint8_t newVolume)
{
    setSmoothedGain(volumeSettings, newVolume, voice::VOL_MAX, MIN_DB);
}
