// sound_module.cpp

#include <esp_log.h>
#include <driver/i2s_std.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "sound_module.hpp"

using namespace sound_module;
using namespace midi_module;
static const char *TAG = "SOUND_MODULE";

SoundModule::SoundModule(const SoundConfig &config)
    : config(config), buffer(config.bufferSize)
{

    soundPool.reserve(config.maxPoliphony);
    for (size_t i = 0; i < config.maxPoliphony; ++i)
    {
        soundPool.emplace_back(config.sampleRate, state.settingsBpm); // ✅ safe and direct
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
        .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO),
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
            8192,
            this,
            configMAX_PRIORITIES - 1,
            &audioTask,
            1 // core 1
        );
    }
}

void SoundModule::handle_note(const NoteMessage &msg)
{
    std::lock_guard<std::mutex> lock(activeSoundsMutex); // 🔒 lock

    for (auto &voice : voices)
    {
        if (msg.on)
        {
            auto *activeSound = allocateSound();
            if (activeSound)
            {
                voice.noteOn(activeSound, msg.channel, msg.note, msg.velocity);
                // ESP_LOGI(TAG, "Handle note");
            }
            else
            {
                ESP_LOGI(TAG, "All sound are taken");
            }
        }
        else
            voice.noteOff(msg.channel, msg.note);
    }
}
void SoundModule::process()
{
    size_t num_samples = config.bufferSize;

    float volumeScale = static_cast<float>(state.masterVolume) / 255.0f;
    {
        std::lock_guard<std::mutex> lock(activeSoundsMutex); // 🔒 protect voices

        for (size_t i = 0; i < num_samples; ++i)
        {
            float mix = 0.0f;
            int activeCount = 0;

            for (auto &voice : voices)
            {
                float sample = voice.getSample(); // mono float

                if (sample != 0.0f)
                {
                    mix += sample;
                    activeCount++;
                }
            }

            if (activeCount > 0)
            {
                mix /= static_cast<float>(activeCount);
            }

            // int16_t intSample = static_cast<int16_t>(mix  * volumeScale);
            int16_t intSample = static_cast<int16_t>(mix * config.amplitude * volumeScale);

            // Duplicate mono sample to both channels
            buffer[i] = intSample; // Left
            // buffer[2 * i + 1] = intSample; // Right
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
    }
}

void SoundModule::updateBpmSetting()
{
    uint16_t bpm = state.usesSettingsBmp ? state.settingsBpm : state.midiBpm;
    for (auto &voice : voices)
    {
        voice.setBpm(bpm);
    }
    for (auto &s : soundPool)
    {
        s.setBpm(bpm);
    }
}

Sound *SoundModule::allocateSound()
{
    for (auto &s : soundPool)
    {
        if (!s.isPlaying())
            return &s;
    }
    // All slots full
    return nullptr;
}
