// voice.cpp
#include "voice/voice.hpp"
#include <cmath>
#include <esp_log.h>

using namespace sound_module;
static const char *TAG = "Voice";

// Find an active Sound by MIDI note
Sound *Voice::find_note_to_release(uint8_t midi_note)
{
    for (auto *s : activeSounds)
    {
        if (s->isNoteOn() && s->midi_note == midi_note)
            return s;
    }
    return nullptr;
}

// Note on: trigger new sound, retrigger LFOs, and envelope
void Voice::noteOn(Sound *sound, uint8_t ch, uint8_t midi_note, uint8_t velocity)
{
    if (ch != midi_channel)
        return;

    // 1. Skip if this note is already active
    for (auto *s : activeSounds)
    {
        if (s->midi_note == midi_note && s->isNoteOn())
        {
            // Note is already playing – ignore new noteOn
            return;
        }
    }

    sound->setPwm(oscillatorSettings.pwm);
    sound->setShape(oscillatorSettings.shape);
    sound->setSync(oscillatorSettings.syncOn);
    sound->envelope.setAttack(envelopeSettings.attack);
    sound->envelope.setDecay(envelopeSettings.decay);
    sound->envelope.setSustain(envelopeSettings.sustain);
    sound->envelope.setRelease(envelopeSettings.release);
    float base_freq = midi_note_freq[midi_note];
    sound->noteOn(base_freq, velocity, midi_note);
    activeSounds.push_back(sound);

    ESP_LOGI(TAG, "Sound added to voice, new count %d", activeSounds.size());
}

// Note off: release matching sound and envelope
void Voice::noteOff(uint8_t ch, uint8_t midi_note)
{
    if (ch != midi_channel)
        return;

    Sound *match = find_note_to_release(midi_note);
    if (match)
    {
        match->noteOff();
    }
}

// Turn off all notes immediately
void Voice::all_notes_off()
{
    for (auto *s : activeSounds)
    {
        s->noteOff();
        s->envelope.gateOff();
    }
}
