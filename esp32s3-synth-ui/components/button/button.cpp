#include "button.hpp"
#include "tag.hpp"
using namespace ui;

static void buttonSender(void *arg)
{
    auto button = static_cast<Button *>(arg);
    while (1)
    {
        vTaskSuspend(button->handle);
        button->ping();
    }
}

IRAM_ATTR void ButtonHandler::button_handler(void *arg)
{
    auto button = static_cast<Button *>(arg);
    xTaskResumeFromISR(button->handle);
};

esp_err_t Button::init(gpio_num_t pin, uint8_t number, ButtonListener listener)
{
    this->state = false;
    this->listener = listener;
    this->pin = pin;
    this->number = number;
    auto config = createConfig(pin);

    ESP_RETURN_ON_ERROR(gpio_config(&config), TAG, "config button");

    ESP_LOGI(TAG, "gpio wakeup source is ready");
    return this->install();
};

esp_err_t Button::install()
{
    gpio_install_isr_service(0);
    ESP_RETURN_ON_ERROR(gpio_isr_handler_add(pin, ButtonHandler::button_handler, this), TAG, "gpio_isr_handler_add");
    xTaskCreate(buttonSender, "buttonSender", 3 * 1024, this, configMAX_PRIORITIES - 2, &handle);
    return ESP_OK;
};

esp_err_t Button::uninstall()
{
    gpio_uninstall_isr_service();
    vTaskDelete(handle);
    return ESP_OK;
};

void Button::toggle(bool value)
{
    state = !state;
    listener(number, state);
};

void Button::ping()
{
    bool value = !gpio_get_level(pin);
    bool isSameValue = this->state == value;
    if (!isSameValue)
    {
        TickType_t now = xTaskGetTickCount();
        // for button down debounce is bigger then button up
        uint8_t k = this->state ? BUTTON_DEBOUNCE_MILLIS_OFF : BUTTON_DEBOUNCE_MILLIS_ON;
        bool debounceOver = now > this->next + k / portTICK_PERIOD_MS;
        if (debounceOver)
        {
            this->next = now;
            state = value;
            listener(number, state);
        }
    }
}
