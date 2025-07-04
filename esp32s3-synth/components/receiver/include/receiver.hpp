#pragma once
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <driver/i2c_slave.h>
#include <esp_err.h>
#include <cstdint>
#include <variant>
#include <vector>
#include "protocol.hpp"

namespace protocol
{

    struct ReceiverConfig
    {
        gpio_num_t sda_pin;       ///< I2C SDA pin
        gpio_num_t scl_pin;       ///< I2C SCL pin
        i2c_port_t i2c_port;      ///< I2C port (default: 0)
        uint8_t receiver_address; ///< SSD1306 I2C address
    };

    class Receiver
    {
    private:
        ReceiverConfig config;
        bool isConnected;
        UpdateCallback callback;
        i2c_slave_dev_handle_t device;

    public:
        explicit Receiver(const ReceiverConfig &config) : config(config) {}
        esp_err_t init(UpdateCallback updateCallback);
        QueueHandle_t receiveQueue = nullptr;
        TaskHandle_t receiveTaskHandle = nullptr;
        void receiveTask();
    };

}
