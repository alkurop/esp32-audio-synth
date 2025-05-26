// voice.hpp
#pragma once
#include <cstdint>
#include <vector>
#include "note_freq_table.hpp"
#include "utils.hpp"
#include "sound.hpp"
#include "envelope.hpp"

namespace sound_module
{

    /**
     * Voice: manages polyphonic Sounds with a shared ADSR envelope.
     * Polyphony count and sample rate are specified at construction.
     */
    class Voice
    {
    public:
        /**
         * Construct a voice engine.
         * @param sample_rate Audio sample rate in Hz.
         * @param max_polyphony Maximum simultaneous notes.
         */
        Voice(uint32_t sample_rate, size_t max_polyphony);

        /**
         * Note on/off handlers.
         * velocity: normalized 0.0–1.0
         */
        void note_on(uint8_t channel, uint8_t midi_note, float velocity);
        void note_off(uint8_t channel, uint8_t midi_note);

        /**
         * Generate the next mixed sample for this voice.
         * @return Sample amplitude in [-1.0, 1.0]
         */
        float get_sample();

        // Voice-level controls
        void set_volume(float v) { volume = v; }
        void set_pitch_shift(float sem) { pitch_shift = sem; }
        void set_transpose(int semitones) { transpose_semitones = semitones; }
        void set_midi_channel(uint8_t ch)
        {
            midi_channel = ch;
            all_notes_off();
        }

        // Access the shared ADSR envelope for parameter changes
        Envelope &envelope() { return amp_env; }
        const Envelope &envelope() const { return amp_env; }

    private:
        uint32_t sample_rate;    // in Hz
        size_t midi_channel = 0; // 0–15
        size_t max_polyphony;    // number of simultaneous sounds
        float volume = 1.0f;
        float pitch_shift = 0.0f;
        uint16_t transpose_semitones = 0;

        std::vector<Sound> sounds; // dynamic polyphony set by constructor
        Envelope amp_env;          // shared ADSR envelope

        // Helpers for voice allocation
        Sound *find_available_slot();
        Sound *find_active_note(uint8_t midi_note);
        void all_notes_off();
    };

} // namespace sound_module
