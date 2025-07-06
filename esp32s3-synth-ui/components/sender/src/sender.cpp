#include <esp_log.h>
#include "sender.hpp"
#include "serialize.hpp"

#define TAG "Sender"

using namespace protocol;

Sender::Sender(const SenderConfig &cfg)
    : config(cfg), isConnected(false) {}

esp_err_t Sender::init()
{

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
    const i2c_master_bus_config_t bus_cfg = {
        .i2c_port = config.i2c_port,
        .sda_io_num = config.sda_pin,
        .scl_io_num = config.scl_pin,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags = {.enable_internal_pullup = false}};

    const i2c_device_config_t synthConfig = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,     // using 7-bit addressing
        .device_address = config.receiver_address, // 7-bit slave address (e.g. 0x42)
        .scl_speed_hz = config.clock_speed,
    };
#pragma GCC diagnostic pop
    esp_err_t err;
    err = i2c_new_master_bus(&bus_cfg, &bus_handle);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "i2c_new_master_bus failed: %s", esp_err_to_name(err));
        return err;
    }

    err = i2c_master_bus_add_device(
        bus_handle,
        &synthConfig,
        &dev_handle);

    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "i2c_param_config failed: %s", esp_err_to_name(err));
        return err;
    }
    isConnected = true;
    ESP_LOGI(TAG, "I2C master initialized (port=%d, SDA=%d, SCL=%d, slave=0x%02X)",
             config.i2c_port, config.sda_pin, config.scl_pin, config.receiver_address);
    startSendTask();

    return ESP_OK;
}

void Sender::startSendTask()
{
    // create a queue that holds EventList objects
    queue = xQueueCreate(64, sizeof(EventList *));
    // spawn the worker on Core 0, passing `this` as the parameter
    xTaskCreatePinnedToCore(
        taskEntry,
        "SenderTask",
        8 * 1024,
        this, // pvParameters
        configMAX_PRIORITIES - 1,
        &taskHandle,
        0);
}

void Sender::taskEntry(void *pv)
{
    auto self = static_cast<Sender *>(pv);
    EventList *evPtr = nullptr;
    while (true)
    {
        // block until someone calls send()
        if (xQueueReceive(self->queue, &evPtr, portMAX_DELAY) == pdTRUE)
        {
            self->doSend(*evPtr);
            delete evPtr; // clean up
        }
    }
}

esp_err_t Sender::send(const EventList &updates)
{
    if (!isConnected)
    {
        ESP_LOGW(TAG, "send() called before init()");
        return ESP_ERR_INVALID_STATE;
    }
    if (auto ptr = new EventList(updates))
    {
        if (xQueueSend(queue, &ptr, 0) != pdTRUE)
        {
            delete ptr;
            return ESP_ERR_NO_MEM;
        }
        return ESP_OK;
    }
    else
    {
        return ESP_ERR_NO_MEM;
    }
}

esp_err_t Sender::doSend(const EventList &events)
{
    // <-- your existing body, unchanged -->
    std::vector<uint8_t> buffer = serializeEvents(events);
    if (buffer.empty())
        return ESP_OK;

    esp_err_t err = i2c_master_transmit(
        dev_handle,
        buffer.data(),
        buffer.size(),
        100 // ms timeout
    );

    if (err != ESP_OK)
        ESP_LOGE(TAG, "i2c_master_transmit failed: %s", esp_err_to_name(err));
    return err;
}
