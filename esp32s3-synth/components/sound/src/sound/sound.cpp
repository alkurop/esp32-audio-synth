#include "sound/sound.hpp"
#include <cmath>
#include <cstdlib> // for std::rand, RAND_MAX

using namespace sound_module;
using namespace protocol;


Sound::Sound(uint32_t sample_rate)
    : sample_rate(sample_rate)
{
}

void Sound::trigger(float frequency, float velocity_in, uint8_t midi_note)
{
    base_frequency = frequency;
    velocity = velocity_in;
    phase = 0.0f;
    phase_increment = base_frequency / sample_rate;
    active = true;
    midi_note = midi_note;
}

void Sound::release()
{
    active = false;
    midi_note = 0;
}

void Sound::set_frequency(float frequency)
{
    phase_increment = frequency / sample_rate;
}

float Sound::get_sample()
{
    if (!active)
        return 0.0f;
    // Generate raw samples for current and next waveform for morphing
    auto generate_wave = [&](protocol::OscillatorShape wf)
    {
        switch (wf)
        {
        case protocol::OscillatorShape::Sine:
            return std::sinf(2.0f * static_cast<float>(M_PI) * phase);
        case protocol::OscillatorShape::Saw:
            return 2.0f * (phase - std::floor(phase + 0.5f));
        case protocol::OscillatorShape::Square:
            return (phase < 0.5f) ? 1.0f : -1.0f;
        case protocol::OscillatorShape::Tri:
            return 2.0f * std::fabsf(2.0f * (phase - std::floor(phase + 0.5f))) - 1.0f;
        case protocol::OscillatorShape::Noise:
            return 2.0f * (static_cast<float>(std::rand()) / RAND_MAX) - 1.0f;
        default:
            return 0.0f;
        }
        return 0.0f;
    };

    // Primary waveform sample
    float rawA = generate_wave(shape);
    // Determine next shape for morph (wrap around _Count)
    uint8_t idx = static_cast<uint8_t>(shape);
    uint8_t nextIdx = (idx + 1) % static_cast<uint8_t>(protocol::OscillatorShape::_Count);
    protocol::OscillatorShape nextShape = static_cast<protocol::OscillatorShape>(nextIdx);
    float rawB = generate_wave(nextShape);

    // Morph between rawA and rawB based on morph (0..MORPH_MAX)
    float t = static_cast<float>(morph) / static_cast<float>(OSCILLATOR_MORPH_MAX);
    float sample = (1.0f - t) * rawA + t * rawB;

    // Apply PWM if square wave in morph targets
    if (shape == protocol::OscillatorShape::Square || nextShape == protocol::OscillatorShape::Square)
    {
        // Compute duty cycle from pwm (0–PWM_MAX) into [0.01 … 0.99]
        float duty = static_cast<float>(pwm) / static_cast<float>(OSCILLATOR_PWM_MAX);
        duty = std::fmax(0.01f, std::fmin(0.99f, duty));
        float squareSample = (phase < duty) ? 1.0f : -1.0f;
        // Blend square contributions according to morph position
        if (shape == protocol::OscillatorShape::Square)
            sample = (1.0f - t) * squareSample + t * rawB;
        else // nextShape is square
            sample = (1.0f - t) * rawA + t * squareSample;
    }

    // Advance phase
    phase += phase_increment;
    if (phase >= 1.0f)
        phase -= 1.0f;

    // Apply velocity
    return sample * velocity;
}

void Sound::set_shape(protocol::OscillatorShape newShape)
{
    shape = newShape;
    // restart waveform on shape change
    phase = 0.0f;
}

void Sound::set_sync(bool sync_on)
{
    sync = sync_on;
    // if enabling sync, restart phase
    if (sync)
        phase = 0.0f;
}
void Sound::set_morph(uint8_t newMorph)
{
    morph = newMorph;
}

void Sound::set_pwm(uint8_t newPwm)
{
    pwm = newPwm;
}

// Configuration getters
protocol::OscillatorShape Sound::get_shape() const
{
    return shape;
}

uint8_t Sound::get_morph() const
{
    return morph;
}

uint8_t Sound::get_pwm() const
{
    return pwm;
}

bool Sound::get_sync() const
{
    return sync;
}
