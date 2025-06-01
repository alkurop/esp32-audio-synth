
#include <esp_log.h>
#include <driver/i2c.h>
#include "receiver.hpp"
#include "serialize.hpp"
#include <driver/i2c_slave.h>

constexpr char *TAG = "Receiver";
using namespace protocol;

bool IRAM_ATTR i2c_slave_receive_cb(i2c_slave_dev_handle_t i2c_slave, const i2c_slave_rx_done_event_data_t *evt_data, void *arg)
{
    auto *self = static_cast<Receiver *>(arg);

    if (!evt_data || evt_data->length == 0)
        return true;

    auto *buf = static_cast<uint8_t *>(heap_caps_malloc(evt_data->length, MALLOC_CAP_8BIT));
    if (!buf)
        return true;

    memcpy(buf, evt_data->buffer, evt_data->length);

    auto *msg = new protocol::IncomingMessage{
        .buffer = buf,
        .length = evt_data->length,
    };

    BaseType_t hpTaskWoken = pdFALSE;
    xQueueSendFromISR(self->receiveQueue, &msg, &hpTaskWoken);
    if (hpTaskWoken)
        portYIELD_FROM_ISR();

    return true;
}

bool IRAM_ATTR i2c_slave_request_cb(i2c_slave_dev_handle_t i2c_slave, const i2c_slave_request_event_data_t *evt_data, void *arg)
{
    Receiver *self = static_cast<Receiver *>(arg);
    auto evt = 0;
    BaseType_t hpTaskWoken = pdFALSE;
    xQueueSendFromISR(self->sendQueue, &evt, &hpTaskWoken);
    if (hpTaskWoken)
        portYIELD_FROM_ISR();
    return false;
}

esp_err_t Receiver::init(UpdateCallback updateCallback, BpmCallback bpmCallback)
{
    this->callback = updateCallback;
    this->bpmCallback = bpmCallback;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

    i2c_slave_config_t bus_cfg = {
        .i2c_port = config.i2c_port,
        .sda_io_num = config.sda_pin,
        .scl_io_num = config.scl_pin,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .send_buf_depth = 100,
        .receive_buf_depth = 100,
        .slave_addr = config.receiver_address,
    };

#pragma GCC diagnostic pop

    esp_err_t err = i2c_new_slave_device(&bus_cfg, &device);
    if (err != ESP_OK)
    {
        ESP_LOGE("Receiver", "Failed to create slave device: %s", esp_err_to_name(err));
        return err;
    }

    i2c_slave_event_callbacks_t cbs = {
        .on_request = i2c_slave_request_cb,
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
                FieldUpdateList updates = protocol::deserializeFieldUpdates(msg->buffer, msg->length);
                heap_caps_free(msg->buffer);
                delete msg;

                // 👇 User-defined callback gets parsed data
                callback(updates);
            }
        }
    }
}

void Receiver::sendTask()
{
    for (;;)
    {
        uint8_t evt;
        if (xQueueReceive(sendQueue, &evt, pdMS_TO_TICKS(10)) == pdTRUE && bpmCallback)
        {
            // 1) Get the latest signed 16-bit BPM
            int16_t bpm = bpmCallback();

            // 2) Pack it into a 2-byte little-endian array
            uint8_t data_buffer[2];
            data_buffer[0] = static_cast<uint8_t>(bpm & 0xFF);        // LSB
            data_buffer[1] = static_cast<uint8_t>((bpm >> 8) & 0xFF); // MSB

            // 3) Declare write_len as uint32_t to match i2c_slave_write’s prototype
            uint32_t write_len = 0;
            esp_err_t err = i2c_slave_write(
                device,              // i2c_slave_dev_handle_t
                data_buffer,         // pointer to our 2-byte payload
                sizeof(data_buffer), // = 2
                &write_len,          // now uint32_t*
                1000                 // timeout in ms
            );

            if (err != ESP_OK)
            {
                ESP_LOGE(TAG, "Write to master failed with code %d", err);
            }
            else if (write_len != sizeof(data_buffer))
            {
                ESP_LOGE(TAG, "Write to master failed with wrong size");
            }
        }
        // Loop again if no event arrived in 10 ticks
    }
}
