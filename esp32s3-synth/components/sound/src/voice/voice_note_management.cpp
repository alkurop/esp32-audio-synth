// voice.cpp
#include "voice/voice.hpp"
#include <cmath>
#include <esp_log.h>

using namespace sound_module;

// Find a free Sound slot
Sound *Voice::find_available_slot()
{
    for (auto &s : sounds)
    {
        if (!s.isPlaying())
            return &s;
    }
    // All slots full
    return nullptr;
}

// Find an active Sound by MIDI note
Sound *Voice::find_note_to_release(uint8_t midi_note)
{
    for (auto &s : sounds)
    {
        if (s.isNoteOn() && s.midi_note == midi_note)
            return &s;
    }
    return nullptr;
}

// Note on: trigger new sound, retrigger LFOs, and envelope
void Voice::noteOn(uint8_t ch, uint8_t midi_note, uint8_t velocity)
{
    if (ch != midi_channel)
        return;

    // 1. Skip if this note is already active
    for (auto &s : sounds)
    {
        if (s.midi_note == midi_note && s.isNoteOn())
        {
            // Note is already playing – ignore new noteOn
            return;
        }
    }

    // 2. Find an available voice (Idle or Releasing)
    Sound *slot = find_available_slot();
    if (!slot)
        return; // all voices are active — skip or add stealing logic later

    float base_freq = midi_note_freq[midi_note];
    slot->noteOn(base_freq, velocity, midi_note);
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
    for (auto &s : sounds)
    {
        s.noteOff();
        s.envelope.gateOff();
    }
}
