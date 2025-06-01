#include <esp_log.h>
#include <driver/i2c.h>
#include "sender.hpp"
#include "serialize.hpp"

static constexpr const char *TAG = "Sender";

using namespace protocol;

Sender::Sender(const SenderConfig &cfg)
    : config(cfg), isConnected(false) {}

esp_err_t Sender::init()
{
    i2c_config_t i2c_conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = config.sda_pin,
        .scl_io_num = config.scl_pin,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master = {
            .clk_speed = 400000,
        },
        .clk_flags = 0,
    };

    esp_err_t err = i2c_param_config(config.i2c_port, &i2c_conf);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "i2c_param_config failed: %s", esp_err_to_name(err));
        return err;
    }

    err = i2c_driver_install(config.i2c_port, I2C_MODE_MASTER, 0, 0, 0);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "i2c_driver_install failed: %s", esp_err_to_name(err));
        return err;
    }

    isConnected = true;
    ESP_LOGI(TAG, "I2C master initialized");
    return ESP_OK;
}

esp_err_t Sender::send(const FieldUpdateList &updates)
{
    if (!isConnected)
    {
        ESP_LOGW(TAG, "send() called before init()");
        return ESP_ERR_INVALID_STATE;
    }

    std::vector<uint8_t> buffer = serializeFieldUpdates(updates);
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (config.receiver_address << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write(cmd, buffer.data(), buffer.size(), true);
    i2c_master_stop(cmd);

    esp_err_t err = i2c_master_cmd_begin(config.i2c_port, cmd, pdMS_TO_TICKS(100));
    i2c_cmd_link_delete(cmd);

    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "i2c_master_cmd_begin failed: %s", esp_err_to_name(err));
    }

    return err;
}

std::variant<FieldUpdate, esp_err_t> Sender::receive(Page page, uint8_t field)
{
    FieldUpdate update;
    // Fill in update from I2C read...
    esp_err_t err = i2c_master_read_from_device(
        config.i2c_port,
        config.receiver_address,
        reinterpret_cast<uint8_t *>(&update),
        sizeof(update),
        pdMS_TO_TICKS(100));

    if (err != ESP_OK)
    {
        return err;
    }

    return update;
}
