// voice.cpp
#include "voice/voice.hpp"
#include "esp_log.h"
#include "cent_pitch_table.hpp"
#include <cmath>

using namespace sound_module;
static const char *TAG = "Voice";

// Constructor: set sample rate, polyphony, initialize sounds and envelope
Voice::Voice(uint8_t voiceIndex, uint32_t sample_rate, uint8_t channel, uint16_t initial_bpm)
    : sampleRate(sample_rate),
      // lfo
      pitchLfo(sample_rate, initial_bpm),
      ampLfo(sample_rate, initial_bpm),
      panLfo(sample_rate, initial_bpm),
      pitchLfoC(pitchLfo, 8, 2 + voiceIndex),
      ampLfoC(ampLfo, 8, 3 + voiceIndex),
      panLfoC(panLfo, 8, 4 + voiceIndex),

      filter(sample_rate, initial_bpm, voiceIndex),
      pitchSettings(),
      midi_channel(channel),
      bpm(initial_bpm),
      volumeSettings()
{
}

void Voice::setBpm(uint16_t bpm)
{
    bpm = bpm;
    ampLfo.setBpm(bpm);
    pitchLfo.setBpm(bpm);
    panLfo.setBpm(bpm);
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
void Voice::updatePitchOffset()

{
    pitchSettings.totalTransposeCents =
        pitchSettings.fine_tuning +
        pitchSettings.transpose_semitones * 100 +
        pitchSettings.transpose_octave * 1200;

    pitchSettings.pitchRatio = sound_module::centsToPitchRatio( pitchSettings.totalTransposeCents);

    ESP_LOGI(TAG, "Updated pitch offset: %d cents pitchRation %f", pitchSettings.totalTransposeCents, pitchSettings.pitchRatio);
}
