// Generate mixed sample and apply envelope
// voice.cpp
#include "voice/voice.hpp"
#include <cmath>
#include "tag.hpp"
#include <esp_log.h>

using namespace sound_module;

float Voice::getSample()
{
    if (volume == 0)
        return 0.0;
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

    // Apply ADSR
    float env_amp = amp_env.next();
    return mix * env_amp * volume;
}
