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
#include <algorithm>

using namespace sound_module;
using namespace protocol;
static const char *TAG = "Sound";

Sound::Sound(uint32_t sample_rate, uint16_t initial_bpm)
    : envelope(sample_rate, initial_bpm), sample_rate(sample_rate) {}

void Sound::noteOn(float frequency, uint8_t velocity_in, uint8_t midi_note_in)
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

void Sound::noteOff()
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
    {
        phase -= 1.0f;
        if (sync)
            phase = 0.0f;
    }

    if (!active && envelope.is_idle())
        return 0.0f;

    auto generate_wave = [&](protocol::OscillatorShape wf, float phaseOffset = 0.0f)
    {
        float ph = phase + phaseOffset;
        if (ph >= 1.0f)
            ph -= 1.0f;

        switch (wf)
        {
        case protocol::OscillatorShape::Sine:
            return interpolateLookup(ph, sineTable);
        case protocol::OscillatorShape::Saw:
        {
            float t = ph;
            float dt = phase_increment; // normalized increment per sample
            float value = 2.0f * t - 1.0f;
            value -= poly_blep(t, dt);
            return value;
        }
        case protocol::OscillatorShape::Square:
        {
            float pw = std::clamp(static_cast<float>(pwm) / static_cast<float>(protocol::OSCILLATOR_PWM_MAX), 0.05f, 0.95f);
            float t = ph;
            float dt = phase_increment;

            float out = (t < pw) ? 1.0f : -1.0f;

            // Apply PolyBLEP at both transitions
            out += poly_blep(t, dt);               // Rising edge
            float t2 = fmodf(t - pw + 1.0f, 1.0f); // Correct wraparound
            out -= poly_blep(t2, dt);              // Falling edge

            return out;
        }
        case protocol::OscillatorShape::Tri:
            return interpolateLookup(ph, triangleTable);
        case protocol::OscillatorShape::Noise:
            return 2.0f * (static_cast<float>(std::rand()) / RAND_MAX) - 1.0f;
        default:
            return 0.0f;
        }
    };

    // Morph between current shape and next shape
    auto shapeA = shape;
    auto shapeB = static_cast<protocol::OscillatorShape>((static_cast<int>(shape) + 1) % static_cast<int>(protocol::OscillatorShape::_Count));

    float morphNorm = std::clamp(static_cast<float>(morph) / static_cast<float>(protocol::OSCILLATOR_MORPH_MAX), 0.0f, 1.0f);
    float waveA = generate_wave(shapeA);
    float waveB = generate_wave(shapeB);

    float morphed = (1.0f - morphNorm) * waveA + morphNorm * waveB;
    return morphed * envelope.next();
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
    velNorm = (static_cast<float>(velocity) / 127.0f) * VELOCITY_GLOBAL_SCALER;
}

void Sound::setBpm(uint16_t bpm)
{
    envelope.setBpm(bpm);
}

bool Sound::isPlaying()
{
    return active || !envelope.is_idle();
}

bool Sound::isNoteOn()
{
    return active;
}
