// voice.cpp
#include "voice.hpp"
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
      cutoffLfo(sampleRate, initial_bpm),
      resonanceLfo(sampleRate, initial_bpm),
      pitchLfoC(pitchLfo, 512),
      ampLfoC(ampLfo, 513),
      cutoffLfoC(cutoffLfo, 514),
      resonanceLfoC(resonanceLfo, 515),
      filter(sample_rate, initial_bpm, voiceIndex),
      pitchSettings(),
      midi_channel(channel),
      bpm(initial_bpm),
      volumeSettings()
{
}

void Voice::setBpm(uint16_t bpm)
{
    ESP_LOGI(TAG, "Voice set pbm %d", bpm);
    bpm = bpm;
    ampLfo.setBpm(bpm);
    pitchLfo.setBpm(bpm);
    cutoffLfo.setBpm(bpm);
    resonanceLfo.setBpm(bpm);
}

void Voice::setMidiChannel(uint8_t midiChannel)
{
    midi_channel = midiChannel;
    all_notes_off();
};

void Voice::setAttack(uint8_t value)
{
    envelopeSettings.attack = value;
    for (auto *s : activeOscillators)
    {
        s->envelope.setAttack(value);
    }
}
void Voice::setDecay(uint8_t value)
{
    envelopeSettings.decay = value;
    for (auto *s : activeOscillators)
    {
        s->envelope.setDecay(value);
    }
}
void Voice::setSustain(uint8_t value)
{
    envelopeSettings.sustain = value;

    for (auto *s : activeOscillators)
    {
        s->envelope.setSustain(value);
    }
}
void Voice::setRelease(uint8_t value)
{
    envelopeSettings.release = value;
    for (auto *s : activeOscillators)
    {
        s->envelope.setRelease(value);
    }
}

void Voice::setOscillatorPwm(uint8_t value)
{
    oscillatorSettings.pwm = value;
    for (auto *s : activeOscillators)
    {
        s->setPwm(value);
    }
}

void Voice::setOscillatorShape(protocol::OscillatorShape value)
{
    oscillatorSettings.shape = value;
    for (auto *o : activeOscillators)
    {
        o->setShape(value);
    }
}

void Voice::updatePitchOffset()

{
    pitchSettings.totalTransposeCents =
        pitchSettings.fine_tuning +
        pitchSettings.transpose_semitones * 100 +
        pitchSettings.transpose_octave * 1200;

    pitchSettings.pitchRatio = centsToPitchRatio(pitchSettings.totalTransposeCents);

    ESP_LOGI(TAG, "Updated pitch offset: %d cents pitchRation %f", pitchSettings.totalTransposeCents, pitchSettings.pitchRatio);
}
