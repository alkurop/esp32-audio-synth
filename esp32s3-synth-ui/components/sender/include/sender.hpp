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
#include "protocol.hpp"

using namespace protocol;

struct SenderConfig
{
    gpio_num_t sda_pin;       ///< I2C SDA pin
    gpio_num_t scl_pin;       ///< I2C SCL pin
    i2c_port_t i2c_port;      ///< I2C port (default: 0)
    uint8_t receiver_address; ///< SSD1306 I2C address
    uint32_t clock_speed;
};

struct ReceiveResult
{
    esp_err_t status;
    FieldUpdate update;
};

class Sender
{
public:
    explicit Sender(const SenderConfig &config);
    esp_err_t init();
    esp_err_t send(const EventList &updates);

private:
    // your existing members
    SenderConfig config;
    bool isConnected = false;
    i2c_master_bus_handle_t bus_handle = nullptr;
    i2c_master_dev_handle_t dev_handle = nullptr;

    // per‐instance queue & task handle
    QueueHandle_t queue = nullptr;
    TaskHandle_t taskHandle = nullptr;

    // the “real” send routine, only called inside the task
    esp_err_t doSend(const EventList &events);

    // FreeRTOS entrypoint (static, but dispatches to the instance)
    static void taskEntry(void *pv);

    // start the task (called from init)
    void startSendTask();
};
