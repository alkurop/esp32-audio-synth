#include "oscillator.hpp"
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

Oscillator::Oscillator(uint32_t sample_rate, uint16_t initial_bpm)
    : envelope(sample_rate, initial_bpm), sample_rate(sample_rate) {}

void Oscillator::noteOn(float frequency, uint8_t velocity_in, uint8_t midi_note_in)
{
    ESP_LOGD(TAG, "Sound trigger freq %f velocity %u note %u", frequency, velocity_in, midi_note);
    setVelocity(velocity_in);
    phase = 0.0f;
    active = true;
    midi_note = midi_note_in;
    envelope.gateOn();
}

void Oscillator::noteOff()
{
    ESP_LOGD(TAG, "Sound release note %u", midi_note);
    active = false;
    envelope.gateOff();
}

void Oscillator::setFrequency(float frequency)
{
    auto p = frequency / sample_rate;
    if (p != phase_increment)
    {
        phase_increment = frequency / sample_rate;
        // ESP_LOGI(TAG, "Pahse inncdement changed freq %f phase_increment %f", frequency, phase_increment);
    }
}

IRAM_ATTR float Oscillator::getSample()
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
            return interpolateLookup(phase, sawTable);
        }
        case protocol::OscillatorShape::Square:
        {
            int pwmIndex = std::clamp(
                static_cast<int>(pwm * PWM_STEPS / 128),
                0,
                static_cast<int>(PWM_STEPS - 1));
            float value = interpolateLookup(phase, pwmSquareTables[pwmIndex]);

            return value;
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

void Oscillator::setShape(protocol::OscillatorShape newShape)
{
    shape = newShape;
    // restart waveform on shape change
    phase = 0.0f;
}

void Oscillator::setPwm(uint8_t newPwm)
{
    pwm = newPwm;
}

void Oscillator::setVelocity(uint8_t velocity)
{
    velNorm = (static_cast<float>(velocity) / 127.0f) * VELOCITY_GLOBAL_SCALER;
}

void Oscillator::setBpm(uint16_t bpm)
{
    envelope.setBpm(bpm);
}

bool Oscillator::isPlaying()
{
    return active || !envelope.is_idle();
}

bool Oscillator::isNoteOn()
{
    return active;
}
