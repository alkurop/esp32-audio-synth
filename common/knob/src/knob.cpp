#include "knob.hpp"
#include <algorithm>
#include <esp_log.h>

constexpr const char *TAG = "MasterKnob";

static constexpr uint8_t DELTA_THRESHOLD = 5;                   
static constexpr TickType_t POLL_INTERVAL = pdMS_TO_TICKS(100); // 100 ms → 10 Hz

using namespace ui;

Knob::Knob(const KnobConfig &config) : config(config) {};

void Knob::init(MasterKnobCallback cb)
{

    callback = std::move(cb);

    //----------------------------------------
    // 2) Configure the oneshot ADC unit
    //----------------------------------------
    // We choose ADC_UNIT_1 since GPIO32..35 are on ADC1.
    adc_oneshot_unit_init_cfg_t initConfig = {
        .unit_id = config.adc_unit,
        .clk_src = ADC_RTC_CLK_SRC_DEFAULT,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };
    esp_err_t err = adc_oneshot_new_unit(&initConfig, &adcUnitHandle);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "adc_oneshot_new_unit failed (%s)", esp_err_to_name(err));
        return;
    }

    // Set each channel to 12-bit width, 11 dB attenuation (0…3.3 V)
    chanConfig.bitwidth = ADC_BITWIDTH_12;
    chanConfig.atten = ADC_ATTEN_DB_12;
    gpio_pullup_dis(config.pin);
    gpio_pulldown_dis(config.pin);
    err = adc_oneshot_config_channel(adcUnitHandle, config.adc_channel, &chanConfig);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "adc_oneshot_config_channel failed (%s)", esp_err_to_name(err));
        return;
    }

    ESP_LOGI(TAG, "MasterKnob reading on pin %d (ADC1_CH%d)", config.pin, config.adc_channel);

    //----------------------------------------
    // 3) Spawn the FreeRTOS “knob task”
    //----------------------------------------
    BaseType_t rc = xTaskCreatePinnedToCore( knobTask, "MasterKnobTask", 4096, this, tskIDLE_PRIORITY + 1, &taskHandle, 1);
    if (rc != pdPASS)
    {
        ESP_LOGE(TAG, "Failed to create MasterKnobTask");
        // Clean up if needed
        adc_oneshot_del_unit(adcUnitHandle);
        adcUnitHandle = nullptr;
    }
}

void Knob::knobTask(void *arg)
{
    auto *self = static_cast<Knob *>(arg);

    while (true)
    {
        //----------------------------------------
        // 1) Do a one-shot ADC read (blocking)
        //----------------------------------------
        int raw = 0;
        esp_err_t err = adc_oneshot_read(self->adcUnitHandle, self->config.adc_channel, &raw);
        if (err != ESP_OK)
        {
            ESP_LOGW(TAG, "adc_oneshot_read failed (%s)", esp_err_to_name(err));
            raw = 0;
        }

        raw = std::clamp(raw, 0, 4095);

        //----------------------------------------
        // 2) Map 0…4095 → 0…255
        //----------------------------------------
        uint8_t v8 = static_cast<uint8_t>((raw * 255) / 4095);

        //----------------------------------------
        // 3) On first sample, send callback immediately
        //----------------------------------------
        if (self->lastRaw < 0)
        {
            self->lastRaw = raw;
            self->last8bit = v8;
            self->callback(v8);
        }
        else
        {
            int delta = static_cast<int>(v8) - static_cast<int>(self->last8bit);
            if (std::abs(delta) >= DELTA_THRESHOLD)
            {
                self->lastRaw = raw;
                self->last8bit = v8;
                self->callback(v8);
            }
        }

        //----------------------------------------
        // 4) Wait before next poll
        //----------------------------------------
        vTaskDelay(POLL_INTERVAL);
    }
}
