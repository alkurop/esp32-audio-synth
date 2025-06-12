#include "sound/sound.hpp"
#include "protocol.hpp"
#include "sound/sine_table.hpp"
#include "sound/saw_tooth_table.hpp"
#include "sound/triangle_table.hpp"
#include "sound/square_table.hpp"
#include "sound/lookup.hpp"
#include <cmath>
#include <cstdlib>   // for std::rand, RAND_MAX
#include "esp_log.h" // for std::rand, RAND_MAX

using namespace sound_module;
using namespace protocol;
static const char *TAG = "Sound";

Sound::Sound(uint32_t sample_rate)
    : sample_rate(sample_rate) {}

void Sound::trigger(float frequency, uint8_t velocity_in, uint8_t midi_note_in)
{
    ESP_LOGD(TAG, "Sound trigger freq %f velocity %u note %u", frequency, velocity_in, midi_note);
    base_frequency = frequency;
    setVelocity(velocity_in);
    phase = 0.0f;
    phase_increment = base_frequency / sample_rate;
    active = true;
    midi_note = midi_note_in;
}

void Sound::release()
{
    ESP_LOGD(TAG, "Sound release note %u", midi_note);
    active = false;
    midi_note = 0;
}

void Sound::set_frequency(float frequency)
{
    phase_increment = frequency / sample_rate;
}

float Sound::get_sample()
{
    // Advance phase
    phase += phase_increment;
    if (phase >= 1.0f)
        phase -= 1.0f;

    if (!active)
        return 0.0f;
    // Generate raw samples for current and next waveform for morphing
    auto generate_wave = [&](protocol::OscillatorShape wf)
    {
        switch (wf)
        {
        case protocol::OscillatorShape::Sine:
            return interpolateLookup(phase, sineTable);
        case protocol::OscillatorShape::Saw:
            return interpolateLookup(phase, sawTable);
        case protocol::OscillatorShape::Square:
            return interpolateLookup(phase, squareTable);
        case protocol::OscillatorShape::Tri:
            return interpolateLookup(phase, triangleTable);
        case protocol::OscillatorShape::Noise:
            return 2.0f * (static_cast<float>(std::rand()) / RAND_MAX) - 1.0f;
        default:
            return 0.0f;
        }
        return 0.0f;
    };

    // Primary waveform sample
    float rawA = generate_wave(shape);
    // // Determine next shape for morph (wrap around _Count)
    // uint8_t idx = static_cast<uint8_t>(shape);
    // uint8_t nextIdx = (idx + 1) % static_cast<uint8_t>(protocol::OscillatorShape::_Count);
    // protocol::OscillatorShape nextShape = static_cast<protocol::OscillatorShape>(nextIdx);
    // float rawB = generate_wave(nextShape);

    // // Morph between rawA and rawB based on morph (0..MORPH_MAX)
    // float t = static_cast<float>(morph) / static_cast<float>(OSCILLATOR_MORPH_MAX);
    // float sample = (1.0f - t) * rawA + t * rawB;

    // // Apply PWM if square wave in morph targets
    // if (shape == protocol::OscillatorShape::Square || nextShape == protocol::OscillatorShape::Square)
    // {
    //     // Compute duty cycle from pwm (0–PWM_MAX) into [0.01 … 0.99]
    //     float duty = static_cast<float>(pwm) / static_cast<float>(OSCILLATOR_PWM_MAX);
    //     duty = std::fmax(0.01f, std::fmin(0.99f, duty));
    //     float squareSample = (phase < duty) ? 1.0f : -1.0f;
    //     // Blend square contributions according to morph position
    //     if (shape == protocol::OscillatorShape::Square)
    //         sample = (1.0f - t) * squareSample + t * rawB;
    //     else // nextShape is square
    //         sample = (1.0f - t) * rawA + t * squareSample;
    // }

    // Apply velocity (amplitude) and return
    return rawA;
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

void Sound::setVelocity(uint8_t velocity)
{
    velNorm = std::pow(static_cast<float>(velocity) / 127.0f, 1.5f) * VELOCITY_GLOBAL_SCALER;
}
