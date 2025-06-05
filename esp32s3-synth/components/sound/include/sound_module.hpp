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
        uint32_t sampleRate; // Audio sample rate in Hz
        size_t tableSize;    // Wavetable resolution
        uint16_t amplitude;  // Peak amplitude for 16-bit audio
        size_t bufferSize;   // Samples per I2S buffer
        size_t numVoices;    // Polyphony
        I2SParams i2s;       // I2S pin configuration
    };

    struct GlobalState
    {
        midi_module::TransportCommand transportState;
        uint8_t masterVolume = 0;
        uint8_t midiBpm = 0;
        uint16_t settingsBpm = 0;
        bool usesSettingsBmp = false;
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
        std::vector<Voice> &getVoices() { return voices; }
        GlobalState &getState() { return state; }
        void updateBpmSetting();

    private:
        SoundConfig config;
        i2s_chan_handle_t txChan;
        std::vector<Voice> voices;
        TaskHandle_t audioTask = nullptr;
        GlobalState state;

        // Internal audio task entry point
        static void audio_task_entry(void *arg);
    };

} // namespace sound_module
