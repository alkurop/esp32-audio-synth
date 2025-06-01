#include <esp_err.h>
#include <esp_log.h>
#include <esp_lcd_panel_io.h>
#include <esp_lcd_panel_ops.h>
#include <esp_lcd_panel_dev.h>
#include <esp_lcd_panel_ssd1306.h>
#include <cstdio> // for std::snprintf
#include <cstring>
#include <algorithm>
#include "display.hpp"

#define LCD_CMD_BITS 8
#define LCD_PARAM_BITS 8

#define LCD_PIXEL_CLOCK_HZ (400 * 1000)

static const char *TAG = "ui::Display";

using namespace ui;
using namespace menu;

Display::Display(const SSD1306Config &cfg)
    : cfg(cfg)
{
}

esp_err_t Display::init()
{
    esp_err_t err;

    ESP_LOGI(TAG, "Configure I2C");
    if ((err = configureI2C()) != ESP_OK)
    {
        ESP_LOGE(TAG, "I2C config failed (%s)", esp_err_to_name(err));
        return err;
    }

    ESP_LOGI(TAG, "Configure SSD1306 panel");
    if ((err = configureSSD1306()) != ESP_OK)
    {
        ESP_LOGE(TAG, "SSD1306 config failed (%s)", esp_err_to_name(err));
        return err;
    }

    ESP_LOGI(TAG, "Configure LVGL");
    if ((err = configureLVGL()) != ESP_OK)
    {
        ESP_LOGE(TAG, "LVGL config failed (%s)", esp_err_to_name(err));
        return err;
    }

    return ESP_OK;
}

esp_err_t Display::configureI2C()
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
    i2c_master_bus_config_t bus_cfg = {
        .i2c_port = cfg.i2c_port,
        .sda_io_num = cfg.sda_pin,
        .scl_io_num = cfg.scl_pin,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags = {.enable_internal_pullup = true}};
#pragma GCC diagnostic pop
    return i2c_new_master_bus(&bus_cfg, &i2c_bus);
}

esp_err_t Display::configureSSD1306()
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
    esp_lcd_panel_io_i2c_config_t io_cfg = {
        .dev_addr = cfg.i2c_addr,
        .control_phase_bytes = 1,
        .dc_bit_offset = 6,
        .lcd_cmd_bits = LCD_CMD_BITS,
        .lcd_param_bits = LCD_PARAM_BITS,
        .scl_speed_hz = LCD_PIXEL_CLOCK_HZ,
    };

    // attach panel IO to the initialized bus
    esp_err_t err = esp_lcd_new_panel_io_i2c(i2c_bus, &io_cfg, &io_handle);
    if (err != ESP_OK)
        return err;

    esp_lcd_panel_dev_config_t panel_cfg = {
        .reset_gpio_num = -1,
        .bits_per_pixel = 1,
    };
    esp_lcd_panel_ssd1306_config_t ssd_cfg = {
        .height = cfg.height};
    panel_cfg.vendor_config = &ssd_cfg;

    err = esp_lcd_new_panel_ssd1306(io_handle, &panel_cfg, &panel_handle);
    if (err != ESP_OK)
        return err;

    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));
#pragma GCC diagnostic pop

    return ESP_OK;
}

esp_err_t Display::configureLVGL()
{

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
    // Initialize LVGL port
    const lvgl_port_cfg_t port_cfg = ESP_LVGL_PORT_INIT_CONFIG();
    lvgl_port_init(&port_cfg);

    // Create the LVGL display
    lvgl_port_display_cfg_t disp_cfg = {
        .io_handle = io_handle,
        .panel_handle = panel_handle,
        .buffer_size = static_cast<uint32_t>(cfg.width * cfg.height),
        .double_buffer = true,
        .hres = cfg.width,
        .vres = cfg.height,
        .monochrome = true,
        .rotation = {.swap_xy = false, .mirror_x = true, .mirror_y = true},
    };
    disp = lvgl_port_add_disp(&disp_cfg);
    if (!disp)
        return ESP_ERR_NO_MEM;

    lv_disp_set_rotation(disp, LV_DISP_ROT_NONE);
#pragma GCC diagnostic pop

    return ESP_OK;
}
