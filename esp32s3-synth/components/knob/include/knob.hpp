#pragma once
#include <functional>
#include <cstdint>
#include <driver/gpio.h>
#include <esp_adc/adc_oneshot.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

namespace ui
{
    struct KnobConfig
    {
        gpio_num_t pin;
        adc_channel_t adc_channel;
        adc_unit_t adc_unit; 

    };

    using MasterKnobCallback = std::function<void(uint8_t)>;
    class Knob
    {
    private:
        KnobConfig config;
        MasterKnobCallback callback;
        TaskHandle_t taskHandle = nullptr;
        adc_oneshot_unit_handle_t adcUnitHandle = nullptr;
        adc_oneshot_chan_cfg_t chanConfig{};
        int lastRaw = -1;        // last 12-bit reading
        uint8_t last8bit = 0xFF; // last reported 0–255 value

        // Polling task
        static void knobTask(void *arg);
        /* data */
    public:
        Knob(const KnobConfig &config);
        void init(MasterKnobCallback callback);
    };

}
