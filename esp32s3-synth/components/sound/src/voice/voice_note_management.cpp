// voice.cpp
#include "voice/voice.hpp"
#include <cmath>
#include "tag.hpp"
#include <esp_log.h>

using namespace sound_module;

// Find a free Sound slot or steal the oldest
Sound &Voice::find_available_slot()
{
    for (auto &s : sounds)
    {
        if (!s.active)
            return s;
    }
    // All slots full — steal the first one
    sounds[0].note_off();
    return sounds[0];
}

// Find an active Sound by MIDI note
Sound *Voice::find_active_note(uint8_t midi_note)
{
    for (auto &s : sounds)
    {
        if (s.active && s.midi_note == midi_note)
            return &s;
    }
    return nullptr;
}

// Note on: trigger new sound, retrigger LFOs, and envelope
void Voice::noteOn(uint8_t ch, uint8_t midi_note, float velocity)
{
    if (ch != midi_channel)
        return;

    // 1) Find an available slot by reference
    Sound &slot = find_available_slot();

    // 2) Compute transposed MIDI note and base frequency
    int transposed = clamp_midi_note(int(midi_note) + pitchSettings.transpose_semitones);
    float base_freq = midi_note_freq[transposed];
    float freq = base_freq * std::pow(2.0f, pitchSettings.fine_tuning / 1200.0f); // fine_tuning in cents

    // 3) Retrigger both LFOs so they start from phase=0 on every new note
    pitch_lfo.reset_phase();
    amp_lfo.reset_phase();

    // 4) Configure envelope for this note
    envelope.gateOn();

    // 5) Start the sound with the calculated frequency and velocity
    slot.note_on(freq, velocity);
}

// Note off: release matching sound and envelope
void Voice::noteOff(uint8_t ch, uint8_t midi_note)
{
    if (ch != midi_channel)
        return;

    Sound *match = find_active_note(midi_note);
    if (match)
    {
        match->note_off();
        envelope.gateOff();
    }
}

// Turn off all notes immediately
void Voice::all_notes_off()
{
    for (auto &s : sounds)
    {
        s.note_off();
        envelope.gateOff();
    }
}
