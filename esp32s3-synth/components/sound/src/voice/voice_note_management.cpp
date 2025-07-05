// voice.cpp
#include "voice.hpp"
#include <cmath>
#include <esp_log.h>

using namespace sound_module;
static const char *TAG = "Voice";

// Find an active Sound by MIDI note
Oscillator *Voice::find_note_to_release(uint8_t midi_note)
{
    for (auto *s : activeOscillators)
    {
        if (s->isNoteOn() && s->midi_note == midi_note)
            return s;
    }
    return nullptr;
}

// Note on: trigger new sound, retrigger LFOs, and envelope
void Voice::noteOn(Oscillator *sound, uint8_t ch, uint8_t midi_note, uint8_t velocity)
{
    if (ch != midi_channel || volumeSettings.volume == 0)
    {
        return;
    }
    bool wasReset = false;
    // 1. Reset if this note is already active
    for (auto *s : activeOscillators)
    {
        if (s->midi_note == midi_note && s->isNoteOn())
        {
            s->reset();
            wasReset = true;
            // ESP_LOGI(TAG, "Note is already playing – reset");
        }
    }

    sound->setPwm(oscillatorSettings.pwm);
    sound->setShape(oscillatorSettings.shape);
    sound->envelope.setAttack(envelopeSettings.attack);
    sound->envelope.setDecay(envelopeSettings.decay);
    sound->envelope.setSustain(envelopeSettings.sustain);
    sound->envelope.setRelease(envelopeSettings.release);
    float base_freq = midi_note_freq[midi_note];
    sound->noteOn(base_freq, velocity, midi_note);
    if (!wasReset)
        activeOscillators.push_back(sound);

    ESP_LOGI(TAG, "Sound added to voice, new count %d", activeOscillators.size());
}

// Note off: release matching sound and envelope
void Voice::noteOff(uint8_t ch, uint8_t midi_note)
{
    if (ch != midi_channel)
        return;

    Oscillator *match = find_note_to_release(midi_note);
    if (match)
    {
        match->noteOff();
    }
}

// Turn off all notes immediately
void Voice::all_notes_off()
{
    for (auto *s : activeOscillators)
    {
        s->noteOff();
        s->envelope.gateOff();
    }
}

void Voice::garbageCollect()
{
    for (auto it = activeOscillators.begin(); it != activeOscillators.end();)
    {
        Oscillator *sound = *it;
        if (!sound->isPlaying())
        {
            it = activeOscillators.erase(it);
        }
        else
        {
            ++it;
        }
    }
}
