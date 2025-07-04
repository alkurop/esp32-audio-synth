#pragma once
#include <functional>
#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <esp_check.h>
#include <esp_sleep.h>
#include <driver/gpio.h>

#define BUTTON_DEBOUNCE_MILLIS_ON 40
#define BUTTON_DEBOUNCE_MILLIS_OFF 60

namespace ButtonHandler
{
    void button_handler(void *arg);
}

namespace ui
{
    using ButtonListener = std::function<void(uint8_t, bool)>;
    class Button
    {
    private:
        ButtonListener listener;
        TickType_t next;
        gpio_num_t pin;
        uint8_t number;
        bool state;
        std::function<void(void)> margin;

    public:
        TaskHandle_t handle;

        esp_err_t init(gpio_num_t pin, uint8_t number, ButtonListener listener);
        bool getState() { return state; }
        void ping();
        void toggle(bool value);

        esp_err_t install();
        esp_err_t uninstall();

        static gpio_config_t createConfig(gpio_num_t pin)
        {
            return {1ULL << pin,
                    GPIO_MODE_INPUT,
                    GPIO_PULLUP_ENABLE,
                    GPIO_PULLDOWN_DISABLE,
                    GPIO_INTR_ANYEDGE};
        };
    };
}
