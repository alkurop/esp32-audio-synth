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

// Note on: trigger new sound and envelope
void Voice::noteOn(uint8_t ch, uint8_t midi_note, float velocity)
{
    if (ch != midi_channel)
        return;

    Sound slot = find_available_slot();
    int transposed = clamp_midi_note(int(midi_note) + transpose_semitones);
    float base_freq = midi_note_freq[transposed];
    float freq = base_freq * std::pow(2.0f, pitch_shift / 12.0f);

    // Configure envelope for this note
    amp_env.noteOn();

    // Start the sound
    slot.note_on(static_cast<uint8_t>(transposed), freq, velocity, sample_rate);
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
        amp_env.noteOff();
    }
}

// Turn off all notes immediately
void Voice::all_notes_off()
{
    for (auto &s : sounds)
    {
        s.note_off();
        amp_env.noteOff();
    }
}
