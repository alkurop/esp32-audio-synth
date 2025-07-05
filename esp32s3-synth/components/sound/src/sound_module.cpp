// sound_module.cpp

#include <esp_log.h>
#include <driver/i2s_std.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_timer.h>
#include "sound_module.hpp"

using namespace sound_module;
using namespace midi_module;
static const char *TAG = "SOUND_MODULE";

SoundModule::SoundModule(const SoundConfig &config)
    : config(config), buffer(config.bufferSize * 2)
{

    oscillatorPool.reserve(config.maxPoliphony);
    for (size_t i = 0; i < config.maxPoliphony; ++i)
    {
        oscillatorPool.emplace_back(config.sampleRate, state.settingsBpm); // ✅ safe and direct
    }

    voices.reserve(config.numVoices);

    for (size_t i = 0; i < config.numVoices; ++i)
    {
        voices.emplace_back(i, config.sampleRate, i, state.settingsBpm);
    }
}

void SoundModule::init()
{
    // Step 1: Create I2S TX channel
    i2s_chan_config_t tx_chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_AUTO, I2S_ROLE_MASTER);
    ESP_ERROR_CHECK(i2s_new_channel(&tx_chan_cfg, &txChan, nullptr));

    // Step 2: Configure TX for standard I2S mode
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
    i2s_std_config_t tx_std_cfg = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(config.sampleRate),
        .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO),
        .gpio_cfg = {
            .mclk = I2S_GPIO_UNUSED,
            .bclk = config.i2s.bclk_io,
            .ws = config.i2s.lrclk_io,
            .dout = config.i2s.data_io,
            .din = I2S_GPIO_UNUSED,
            .invert_flags = {false, false, false},
        },
    };
#pragma GCC diagnostic pop

    ESP_ERROR_CHECK(i2s_channel_init_std_mode(txChan, &tx_std_cfg));
    ESP_ERROR_CHECK(i2s_channel_enable(txChan));

    // Step 3: Launch audio task pinned to core 1
    if (audioTask == nullptr)
    {
        xTaskCreatePinnedToCore(
            audio_task_entry,
            "audio_task",
            8192 * 16,
            this,
            configMAX_PRIORITIES - 1,
            &audioTask,
            0 // core 1
        );
    }
}

void SoundModule::handle_note(const MidiNoteEvent &msg)
{
    std::lock_guard<std::mutex> lock(activeOscillatorsMutex); // 🔒 lock

    for (auto &voice : voices)
    {
        if (msg.isNoteOn())
        {
            auto *activeSound = allocateSound();
            if (activeSound)
            {
                voice.noteOn(activeSound, msg.channel(), msg.note, msg.velocity);
                // ESP_LOGI(TAG, "Handle note");
            }
        }
        else if (msg.isNoteOff())
            voice.noteOff(msg.channel(), msg.note);
    }
}
IRAM_ATTR void SoundModule::process()
{
    size_t num_samples = config.bufferSize;

    {
        std::lock_guard<std::mutex> lock(activeOscillatorsMutex); // 🔒 protect voices
        for (size_t i = 0; i < num_samples; ++i)
        {
            float volumeScale = state.volumeSettings.gain_smoothed.next();

            float mixLeft = 0.0f;
            float mixRight = 0.0f;

            for (auto &voice : voices)
            {
                auto sample = voice.getSample(); // mono float

                if (sample.left != 0.0f || sample.right != 0.0f)
                {
                    mixLeft += sample.left;
                    mixRight += sample.right;
                }
            }
            int16_t intLeft = static_cast<int16_t>(mixLeft * config.amplitude * volumeScale);
            int16_t intRight = static_cast<int16_t>(mixRight * config.amplitude * volumeScale);

            buffer[2 * i] = intLeft;
            buffer[2 * i + 1] = intRight;
        }
    }
    size_t bytes_written;
    i2s_channel_write(txChan, buffer.data(), buffer.size() * sizeof(int16_t), &bytes_written, portMAX_DELAY);
}

void SoundModule::audio_task_entry(void *arg)
{
    auto *self = static_cast<SoundModule *>(arg);
    while (true)
    {
        int64_t start = esp_timer_get_time();
        self->process();
        int64_t elapsed_us = esp_timer_get_time() - start;
        // ESP_LOGI("AUDIO", "Process time: %lld us", elapsed_us);
        // esp_task_wdt_reset(); 👈 allows watchdog to breathe
        // taskYIELD(); // 👈 allows watchdog to breathe
    }
}

void SoundModule::updateBpmSetting()
{
    uint16_t bpm = state.isSynced ? state.midiBpm : state.settingsBpm;
    for (auto &voice : voices)
    {
        voice.setBpm(bpm);
    }
    for (auto &s : oscillatorPool)
    {
        s.setBpm(bpm);
    }
}

Oscillator *SoundModule::allocateSound()
{
    for (auto &s : oscillatorPool)
    {
        if (!s.isPlaying())
            return &s;
    }
    // No free sounds
    return nullptr;
}
