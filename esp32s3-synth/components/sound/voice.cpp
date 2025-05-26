// voice.cpp
#include "voice.hpp"
#include <cmath>
#include "tag.hpp"
#include <esp_log.h>

using namespace sound_module;

// Constructor: set sample rate, polyphony, initialize sounds and envelope
Voice::Voice(uint32_t sample_rate_, size_t max_polyphony_)
    : sample_rate(sample_rate_),
      midi_channel(0),
      max_polyphony(max_polyphony_),
      volume(1.0f),
      pitch_shift(0.0f),
      transpose_semitones(0),
      sounds(),
      amp_env(sample_rate_)
{
    sounds.reserve(max_polyphony);
    for (size_t i = 0; i < max_polyphony; ++i)
    {
        sounds.emplace_back();
    }
}

// Find a free Sound slot or steal the oldest
Sound *Voice::find_available_slot()
{
    for (auto &s : sounds)
    {
        if (!s.active)
            return &s;
    }
    // All slots full — steal the first one
    sounds[0].note_off();
    return &sounds[0];
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
void Voice::note_on(uint8_t ch, uint8_t midi_note, float velocity)
{
    if (ch != midi_channel)
        return;

    Sound *slot = find_available_slot();
    int transposed = clamp_midi_note(int(midi_note) + transpose_semitones);
    float base_freq = midi_note_freq[transposed];
    float freq = base_freq * std::pow(2.0f, pitch_shift / 12.0f);

    // Configure envelope for this note
    amp_env.note_on();

    // Start the sound
    slot->note_on(static_cast<uint8_t>(transposed), freq, velocity, sample_rate);
}

// Note off: release matching sound and envelope
void Voice::note_off(uint8_t ch, uint8_t midi_note)
{
    if (ch != midi_channel)
        return;

    Sound *match = find_active_note(midi_note);
    if (match)
    {
        match->note_off();
        amp_env.note_off();
    }
}

// Generate mixed sample and apply envelope
float Voice::get_sample()
{
    float mix = 0.0f;
    int active_count = 0;

    for (auto &s : sounds)
    {
        if (!s.active)
            continue;
        mix += s.get_sample(sample_rate);
        ++active_count;
    }

    if (active_count > 0)
        mix /= static_cast<float>(active_count);

    // Apply ADSR
    float env_amp = amp_env.next();
    return mix * env_amp * volume;
}

// Turn off all notes immediately
void Voice::all_notes_off()
{
    for (auto &s : sounds)
        s.active = false;
}
