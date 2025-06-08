#include "sound/sound.hpp"
#include <cmath>

using namespace sound_module;
Sound::Sound(uint32_t sample_rate) : sample_rate(sample_rate)
{
}

void Sound::note_on(float frequency, float velocity_in)
{
    base_frequency = frequency;
    velocity = velocity_in;
    phase = 0.0f;
    phase_increment = base_frequency / sample_rate;
    active = true;
}

void Sound::note_off()
{
    active = false;
}

void Sound::set_frequency(float frequency)
{
    phase_increment = frequency / sample_rate;
}

float Sound::get_sample()
{
    if (!active)
        return 0.0f;

    // Simple sine oscillator
    float sample = std::sinf(2.0f * static_cast<float>(M_PI) * phase) * velocity;
    phase += phase_increment;
    if (phase >= 1.0f)
        phase -= 1.0f;

    return sample;
}
