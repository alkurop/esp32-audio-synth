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

Sound::Sound(uint32_t sample_rate, uint16_t initial_bpm)
    : envelope(sample_rate, initial_bpm), sample_rate(sample_rate) {}

void Sound::trigger(float frequency, uint8_t velocity_in, uint8_t midi_note_in)
{
    ESP_LOGD(TAG, "Sound trigger freq %f velocity %u note %u", frequency, velocity_in, midi_note);
    base_frequency = frequency;
    setVelocity(velocity_in);
    phase = 0.0f;
    phase_increment = base_frequency / sample_rate;
    active = true;
    midi_note = midi_note_in;
    envelope.gateOn();
}

void Sound::release()
{
    ESP_LOGD(TAG, "Sound release note %u", midi_note);
    active = false;
    envelope.gateOff();
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

    if (!active && envelope.is_idle())
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
    return rawA * envelope.next();
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

void Sound::setVelocity(uint8_t velocity)
{
    velNorm = std::pow(static_cast<float>(velocity) / 127.0f, 1.5f) * VELOCITY_GLOBAL_SCALER;
}

void Sound::setBpm(uint16_t bpm)
{
    envelope.setBpm(bpm);
}

bool Sound::isActive()
{
    return active || !envelope.is_idle();
}
