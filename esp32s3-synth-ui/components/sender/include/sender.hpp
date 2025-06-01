#pragma once
#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/i2c_master.h>
#include <esp_err.h>
#include <cstdint>
#include <variant>
#include <vector>
#include "menu_struct.hpp"

namespace protocol
{
    struct SenderConfig
    {
        gpio_num_t sda_pin;       ///< I2C SDA pin
        gpio_num_t scl_pin;       ///< I2C SCL pin
        i2c_port_t i2c_port;      ///< I2C port (default: 0)
        uint8_t receiver_address; ///< SSD1306 I2C address
    };

    struct ReceiveResult
    {
        esp_err_t status;
        FieldUpdate update;
    };

    class Sender
    {
    private:
        SenderConfig config;
        bool isConnected;

    public:
        explicit Sender(const SenderConfig &config);
        esp_err_t init();
        esp_err_t send(const FieldUpdateList &updates);
        std::variant<FieldUpdate, esp_err_t> receive(Page page, uint8_t field);
    };

    template <class... Ts>
    struct overloaded : Ts...
    {
        using Ts::operator()...;
    };
    template <class... Ts>
    overloaded(Ts...) -> overloaded<Ts...>;
}
