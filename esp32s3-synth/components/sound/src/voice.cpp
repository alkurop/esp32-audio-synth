// voice.cpp
#include "voice.hpp"
#include <cmath>
#include "tag.hpp"
#include <esp_log.h>

using namespace sound_module;

// Constructor: set sample rate, polyphony, initialize sounds and envelope
Voice::Voice(uint32_t sample_rate, size_t max_polyphony)
    : sample_rate(sample_rate),
      midi_channel(0),
      max_polyphony(max_polyphony),
      volume(0.0f),
      pitch_shift(0.0f),
      transpose_semitones(0),
      sounds(),
      amp_env(sample_rate)
{
    sounds.reserve(max_polyphony);
    for (size_t i = 0; i < max_polyphony; ++i)
    {
        sounds.emplace_back();
    }
}


void Voice::setBpm(uint16_t bpm)
{
    // TODO where do we use bpm?
}
