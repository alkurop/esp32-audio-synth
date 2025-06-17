// voice.hpp
#pragma once
#include <cstdint>
#include <vector>
#include <functional>
#include <optional>
#include "note_freq_table.hpp"
#include "../utils.hpp"
#include "sound/sound.hpp"
#include "menu_struct.hpp"
#include "lfo.hpp"
#include "cached_lfo.hpp"
#include "nodes/filter.hpp"
#include "protocol.hpp"

using namespace protocol;
namespace sound_module
{

    /**
     * Voice: manages polyphonic Sounds with a shared ADSR envelope.
     * Polyphony count and sample rate are specified at construction.
     */
    class Voice
    {
    public:
        using SoundAllocator = std::function<std::optional<Sound *>()>;

        /**
         * Construct a voice engine.
         * @param sample_rate Audio sample rate in Hz.
         * @param max_polyphony Maximum simultaneous notes.
         */
        Voice(uint8_t index, uint32_t sample_rate, uint8_t channel, uint16_t initial_bpm, SoundAllocator allocator);

        /**
         * Note on/off handlers.
         * velocity: normalized 0.0–1.0
         */
        void noteOn(uint8_t channel, uint8_t midi_note, uint8_t velocity);
        void noteOff(uint8_t channel, uint8_t midi_note);

        /**
         * Generate the next mixed sample for this voice.
         * @return Sample amplitude in [-1.0, 1.0]
         */
        float getSample();

        // Voice-level controls
        void setVolume(uint8_t volume);
        void setMidiChannel(uint8_t ch);
        void setBpm(uint16_t bpm);

        // Envelope settings per sound
        void setAttack(uint8_t value);
        void setDecay(uint8_t value);
        void setSustain(uint8_t value);
        void setRelease(uint8_t value);

        void setOscillatorShape(protocol::OscillatorShape value);
        void setOscillatorPwm(uint8_t value);
        void setOscillatorSync(bool value);

        const uint16_t sampleRate;

        LFO pitchLfo;
        LFO ampLfo;
        LFO panLfo;

        CachedLFO pitchLfoC;
        CachedLFO ampLfoC;
        CachedLFO panLfoC;
        
        Filter filter;

    private:
        size_t midi_channel = 0;
        uint16_t bpm;

        voice::VolumeSettings volumeSettings;
        voice::PitchSettings pitchSettings;
        voice::EnvelopeSettings envelopeSettings;
        voice::OscillatorSettings oscillatorSettings;

        SoundAllocator allocateSound;
        std::vector<Sound *> activeSounds;

        Sound *find_note_to_release(uint8_t midi_note); // can be a nullptr
        Sound *find_available_slot();

        void all_notes_off();
    };

} // namespace sound_module
