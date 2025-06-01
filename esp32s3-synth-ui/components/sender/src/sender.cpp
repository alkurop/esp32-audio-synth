#include <esp_log.h>
#include "sender.hpp"
#include "serialize.hpp"

static constexpr const char *TAG = "Sender";

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
    };

    const i2c_device_config_t synthConfig = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,     // using 7-bit addressing
        .device_address = config.receiver_address, // 7-bit slave address (e.g. 0x42)
        .scl_speed_hz = config.clock_speed,        // e.g. 100000 for 100 kHz
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

    return ESP_OK;
}

esp_err_t Sender::send(const FieldUpdateList &updates)
{
    if (!isConnected)
    {
        ESP_LOGW(TAG, "send() called before init()");
        return ESP_ERR_INVALID_STATE;
    }

    // 1) Serialize your updates into a byte buffer
    std::vector<uint8_t> buffer = serializeFieldUpdates(updates);
    if (buffer.empty())
    {
        // Nothing to send
        return ESP_OK;
    }

    // 2) Call i2c_master_transmit() with your device handle, buffer, length, and timeout (in ms)
    //    dev_handle was set in init() via i2c_master_bus_add_device(...)
    esp_err_t err = i2c_master_transmit(
        dev_handle,    // i2c_master_dev_handle_t
        buffer.data(), // pointer to bytes to write
        buffer.size(), // number of bytes to send
        100            // timeout in milliseconds
    );

    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "i2c_master_transmit failed: %s", esp_err_to_name(err));
    }
    else
    {
        ESP_LOGI(TAG, "i2c_master_transmit success size %d", buffer.size());
    }

    return err;
}

std::variant<FieldUpdate, esp_err_t> Sender::receiveBpm()
{
    if (!isConnected)
    {
        // init() was never called or failed
        return ESP_ERR_INVALID_STATE;
    }

    // 1) Prepare a 2-byte buffer
    uint8_t buf[2] = {0, 0};

    // 2) Do a “pure read” by using transmit_receive with write_size = 0
    //    Under the hood this issues: START → (addr << 1) | READ → read 2 bytes → STOP
    esp_err_t err = i2c_master_transmit_receive(
        dev_handle,  // handle from i2c_master_bus_add_device()
        nullptr,     // no write-phase
        0,           // write_size = 0
        buf,         // read_buffer
        sizeof(buf), // read_size = 2
        100          // timeout = 100 ms
    );

    if (err != ESP_OK)
    {
        return err;
    }

    // 3) Reconstruct little-endian uint16_t
    uint16_t value = static_cast<uint16_t>(buf[0]) |
                     static_cast<uint16_t>(buf[1]) << 8;

    return value;
}
