#include "sound/sound.hpp"
#include "protocol.hpp"
#include "sine_table.hpp"
#include "saw_tooth_table.hpp"
#include "triangle_table.hpp"
#include "square_table.hpp"
#include "lookup.hpp"
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

void Sound::setFrequency(float frequency)
{
    
    phase_increment = frequency / sample_rate;
}

float Sound::getSample()
{
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
        {
            float t = phase;
            float dt = phase_increment; // normalized increment per sample
            float value = 2.0f * t - 1.0f;
            value -= poly_blep(t, dt);
            return value;
        }
        case protocol::OscillatorShape::Square:
        {
            float pw = std::clamp(static_cast<float>(pwm) / static_cast<float>(protocol::OSCILLATOR_PWM_MAX), 0.05f, 0.95f);
            float t = phase;
            float dt = phase_increment;

            float out = (t < pw) ? 1.0f : -1.0f;

            // Apply PolyBLEP at both transitions
            out += poly_blep(t, dt);               // Rising edge
            float t2 = fmodf(t - pw + 1.0f, 1.0f); // Correct wraparound
            out -= poly_blep(t2, dt);              // Falling edge

            return out;
        }
        case protocol::OscillatorShape::Tri:
            return interpolateLookup(phase, triangleTable);
        case protocol::OscillatorShape::Noise:
            return 2.0f * (static_cast<float>(std::rand()) / RAND_MAX) - 1.0f;
        default:
            return 0.0f;
        }
        return 0.0f;
    };

    auto waveform = generate_wave(shape);

    // return waveform;
    return waveform * envelope.next();
}

void Sound::setShape(protocol::OscillatorShape newShape)
{
    shape = newShape;
    // restart waveform on shape change
    phase = 0.0f;
}

void Sound::setSync(bool sync_on)
{
    sync = sync_on;
    // if enabling sync, restart phase
    if (sync)
        phase = 0.0f;
}

void Sound::setPwm(uint8_t newPwm)
{
    pwm = newPwm;
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
