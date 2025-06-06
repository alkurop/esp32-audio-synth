// voice.hpp
#pragma once
#include <cstdint>
#include <vector>
#include "note_freq_table.hpp"
#include "../utils.hpp"
#include "sound.hpp"
#include "../nodes/envelope.hpp"

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
        void noteOn(uint8_t channel, uint8_t midi_note, float velocity);
        void noteOff(uint8_t channel, uint8_t midi_note);

        /**
         * Generate the next mixed sample for this voice.
         * @return Sample amplitude in [-1.0, 1.0]
         */
        float getSample();

        // Voice-level controls
        void setVolume(float v) { volume = v; }
        void setPitchShift(float sem) { pitch_shift = sem; }
        void setTranspose(int semitones) { transpose_semitones = semitones; }
        void setMidiChannel(uint8_t ch)
        {
            midi_channel = ch;
            all_notes_off();
        }

        void setBpm(uint16_t bpm);

        // Access the shared ADSR envelope for parameter changes
        Envelope &envelope() { return amp_env; }

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
        Sound &find_available_slot();
        Sound *find_active_note(uint8_t midi_note); // can be a nullptr
        void all_notes_off();
    };

} // namespace sound_module
