// voice.cpp
#include "voice/voice.hpp"
#include <cmath>
#include "tag.hpp"
#include <esp_log.h>

using namespace sound_module;

// Constructor: set sample rate, polyphony, initialize sounds and envelope
Voice::Voice(uint32_t sample_rate, size_t max_polyphony, uint8_t channel, uint16_t initial_bpm)
    : config({.sample_rate = sample_rate, .max_polyphony = max_polyphony}),
      volumeSettings(),
      pitchSettings(),
      envelope(sample_rate, initial_bpm),
      pitch_lfo(sample_rate, initial_bpm),
      amp_lfo(sample_rate, initial_bpm),
      midi_channel(channel),
      sounds()

{
    sounds.reserve(max_polyphony);
    for (size_t i = 0; i < max_polyphony; ++i)
    {
        sounds.emplace_back(config.sample_rate);
    }
}

void Voice::setBpm(uint16_t bpm)
{
    bpm = bpm;
    envelope.setBpm(bpm);
    amp_lfo.setBpm(bpm);
    pitch_lfo.setBpm(bpm);
}

void Voice::setMidiChannel(uint8_t midiChannel)
{
    midi_channel = midiChannel;
    all_notes_off();
};
