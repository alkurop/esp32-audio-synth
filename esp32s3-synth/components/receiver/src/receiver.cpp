
#include <esp_log.h>
#include "receiver.hpp"
#include "serialize.hpp"
#include <cstring>
#define TAG "Receiver"
using namespace protocol;

static bool i2c_slave_receive_cb(
    i2c_slave_dev_handle_t i2c_slave,
    const i2c_slave_rx_done_event_data_t *evt_data,
    void *arg)
{
    auto *self = static_cast<Receiver *>(arg);

    // 1) Check for valid incoming data
    if (!evt_data || evt_data->length == 0)
    {
        return false; // nothing to do
    }

    // 2) Allocate a buffer of exactly evt_data->size bytes in DRAM
    size_t len = evt_data->length;
    auto *buf = static_cast<uint8_t *>(
        heap_caps_malloc(len, MALLOC_CAP_8BIT));
    if (!buf)
    {
        return false; // allocation failed
    }

    // 3) Copy the incoming bytes into our buffer
    memcpy(buf, evt_data->buffer, len);

    // 4) Create a new IncomingMessage and store both buffer and length
    auto *msg = new protocol::IncomingMessage{
        .buffer = buf,
        .length = len, // <— use 'len', not 0
    };

    // 5) Push the pointer to our receiveQueue (ISR-safe)
    BaseType_t hpTaskWoken = pdFALSE;
    xQueueSendFromISR(self->receiveQueue, &msg, &hpTaskWoken);
    if (hpTaskWoken)
    {
        portYIELD_FROM_ISR();
    }
    return false;
}

esp_err_t Receiver::init(UpdateCallback updateCallback)
{
    this->callback = updateCallback;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

    i2c_slave_config_t bus_cfg = {
        .i2c_port = config.i2c_port,
        .sda_io_num = config.sda_pin,
        .scl_io_num = config.scl_pin,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .send_buf_depth = 4000,
        .receive_buf_depth = 4000,
        .slave_addr = config.receiver_address,
        .flags = {.enable_internal_pullup = false}

    };

#pragma GCC diagnostic pop

    esp_err_t err = i2c_new_slave_device(&bus_cfg, &device);
    if (err != ESP_OK)
    {
        ESP_LOGE("Receiver", "Failed to create slave device: %s", esp_err_to_name(err));
        return err;
    }

    i2c_slave_event_callbacks_t cbs = {
        .on_request = nullptr,
        .on_receive = i2c_slave_receive_cb,
    };

    err = i2c_slave_register_event_callbacks(device, &cbs, this);
    if (err != ESP_OK)
    {
        ESP_LOGE("Receiver", "Failed to register slave callbacks: %s", esp_err_to_name(err));
        return err;
    }

    isConnected = true;

    ESP_LOGI("Receiver", "I2C slave initialized on port %d", config.i2c_port);
    receiveQueue = xQueueCreate(8, sizeof(FieldUpdateList *));

    if (!receiveQueue)
    {
        ESP_LOGE("Receiver", "Failed to create update queue");
        return ESP_ERR_NO_MEM;
    }
    xTaskCreatePinnedToCore([](void *arg)
                            { static_cast<Receiver *>(arg)->receiveTask(); }, "receiver_rx", 8192, this, 5, &receiveTaskHandle, 1); // Core 1

    return ESP_OK;
}

void Receiver::receiveTask()
{
    while (true)
    {
        protocol::IncomingMessage *msg = nullptr;
        if (xQueueReceive(receiveQueue, &msg, portMAX_DELAY) == pdTRUE)
        {

            if (msg && msg->buffer)
            {
                // ⛳ Deserialization happens here!
                EventList events = protocol::deserializeEvents(msg->buffer, msg->length);
                // 👇 User-defined callback gets parsed data
                callback(events);
                heap_caps_free(msg->buffer);
                delete msg;
            }
        }
    }
}
