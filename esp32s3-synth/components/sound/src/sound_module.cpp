// sound_module.cpp

#include <esp_log.h>
#include <driver/i2s_std.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "sound_module.hpp"
#include "utils.hpp"
#include "stereo.hpp"

using namespace sound_module;
using namespace midi_module;

SoundModule::SoundModule(const SoundConfig &config)
    : config(config)
{

    soundPool.reserve(config.maxPoliphony);
    for (size_t i = 0; i < config.maxPoliphony; ++i)
    {
        soundPool.emplace_back(config.sampleRate, state.settingsBpm); // ✅ safe and direct
    }

    voices.reserve(config.numVoices);

    auto allocator = [this]() -> std::optional<Sound *>
    {
        return allocateSound();
    };
    for (size_t i = 0; i < config.numVoices; ++i)
    {
        voices.emplace_back(i, config.sampleRate, i, state.settingsBpm, allocator);
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
    for (auto &voice : voices)
    {
        if (msg.on)
            voice.noteOn(msg.channel, msg.note, msg.velocity);
        else
            voice.noteOff(msg.channel, msg.note);
    }
}

void SoundModule::process()
{
    size_t num_samples = config.bufferSize;
    std::vector<int16_t> buffer(num_samples * 2); // stereo buffer

    float volumeScale = static_cast<float>(state.masterVolume) / 255.0f;

    for (size_t i = 0; i < num_samples; ++i)
    {
        float mixL = 0.0f;
        float mixR = 0.0f;
        int activeCount = 0;

        for (auto &voice : voices)
        {
            Stereo s = voice.getSample();

            if (s.left != 0.0f || s.right != 0.0f)
            {
                mixL += s.left;
                mixR += s.right;
                activeCount++;
            }
        }

        if (activeCount > 0)
        {
            mixL /= static_cast<float>(activeCount);
            mixR /= static_cast<float>(activeCount);
        }

        int16_t sampleL = static_cast<int16_t>(mixL * config.amplitude * volumeScale);
        int16_t sampleR = static_cast<int16_t>(mixR * config.amplitude * volumeScale);

        buffer[2 * i] = sampleL;
        buffer[2 * i + 1] = sampleR;
    }

    size_t bytes_written;
    i2s_channel_write(txChan, buffer.data(), buffer.size() * sizeof(int16_t), &bytes_written, portMAX_DELAY);
}

void SoundModule::audio_task_entry(void *arg)
{
    auto *self = static_cast<SoundModule *>(arg);
    while (true)
    {
        // int64_t start = esp_timer_get_time();
        self->process();
        // int64_t elapsed_us = esp_timer_get_time() - start;
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
