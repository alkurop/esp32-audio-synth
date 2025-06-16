// voice.cpp
#include "voice/voice.hpp"
#include <cmath>

using namespace sound_module;

// Constructor: set sample rate, polyphony, initialize sounds and envelope
Voice::Voice(uint32_t sample_rate, uint8_t channel, uint16_t initial_bpm, SoundAllocator allocator)
    : sampleRate(sample_rate),
      pitch_lfo(sample_rate, initial_bpm),
      amp_lfo(sample_rate, initial_bpm),
      filter(sample_rate, initial_bpm),
      midi_channel(channel),
      bpm(initial_bpm),
      volumeSettings(),
      pitchSettings(),
      allocateSound(allocator)
{
}

void Voice::setBpm(uint16_t bpm)
{
    bpm = bpm;
    amp_lfo.setBpm(bpm);
    pitch_lfo.setBpm(bpm);
    filter.setBpm(bpm);
}

void Voice::setMidiChannel(uint8_t midiChannel)
{
    midi_channel = midiChannel;
    all_notes_off();
};

void Voice::setAttack(uint8_t value)
{
    envelopeSettings.attack = value;
    for (auto *s : activeSounds)
    {
        s->envelope.setAttack(value);
    }
}
void Voice::setDecay(uint8_t value)
{
    envelopeSettings.decay = value;
    for (auto *s : activeSounds)
    {
        s->envelope.setDecay(value);
    }
}
void Voice::setSustain(uint8_t value)
{
    envelopeSettings.sustain = value;

    for (auto *s : activeSounds)
    {
        s->envelope.setSustain(value);
    }
}
void Voice::setRelease(uint8_t value)
{
    envelopeSettings.release = value;
    for (auto *s : activeSounds)
    {
        s->envelope.setRelease(value);
    }
}

void Voice::setOscillatorPwm(uint8_t value)
{
    oscillatorSettings.pwm = value;
    for (auto *s : activeSounds)
    {
        s->setPwm(value);
    }
}

void Voice::setOscillatorShape(protocol::OscillatorShape value)
{
    oscillatorSettings.shape = value;
    for (auto *s : activeSounds)
    {
        s->setShape(value);
    }
}

void Voice::setOscillatorSync(bool value)
{
    oscillatorSettings.syncOn = value;
    for (auto *s : activeSounds)
    {
        s->setSync(value);
    }
}

