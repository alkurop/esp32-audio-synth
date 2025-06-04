// sound_module.hpp
#pragma once

#include <cstdint>
#include <vector>
#include <functional>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>
#include <driver/i2s_std.h>
#include "voice.hpp"
#include "midi_parser.hpp"
#include "menu_struct.hpp"


namespace sound_module
{

    // Configuration for I2S pins
    struct I2SParams
    {
        gpio_num_t bclk_io;  // Bit clock
        gpio_num_t lrclk_io; // Word select (left/right clock)
        gpio_num_t data_io;  // Serial data out
    };

    // Main sound engine configuration
    struct SoundConfig
    {
        uint32_t sampleRate = 44100; // Audio sample rate in Hz
        size_t tableSize = 256;     // Wavetable resolution
        uint16_t amplitude = 16000;  // Peak amplitude for 16-bit audio
        size_t bufferSize = 64;     // Samples per I2S buffer
        size_t numVoices = protocol::NUM_VOICES;       // Polyphony
        I2SParams i2s;              // I2S pin configuration
    };

    class SoundModule
    {
    public:
        explicit SoundModule(const SoundConfig &config);
        void init();
        void process();

        // MIDI input handler
        void handle_note(const midi_module::NoteMessage &msg);

        // Access voices for advanced control
        std::vector<Voice> &voices() { return _voices; }
        const std::vector<Voice> &voices() const { return _voices; }

    private:
        SoundConfig _config;
        i2s_chan_handle_t _txChan;
        std::vector<Voice> _voices;
        TaskHandle_t _audioTask = nullptr;

        // Internal audio task entry point
        static void audio_task_entry(void *arg);
    };

} // namespace sound_module
