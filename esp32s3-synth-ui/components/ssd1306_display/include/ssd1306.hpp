#pragma once
#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <driver/i2c_master.h>
#include <array>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-enum-enum-conversion"
#include "lvgl.h"
#include <esp_lvgl_port.h>
#pragma GCC diagnostic pop

#include "menu_struct.hpp"

namespace ui
{

    /**
     * Configuration for SSD1306 display
     */
    struct SSD1306Config
    {
        gpio_num_t sda_pin;           ///< I2C SDA pin
        gpio_num_t scl_pin;           ///< I2C SCL pin
        uint8_t width;                ///< Display width in pixels
        uint8_t height;               ///< Display height in pixels
        uint8_t i2c_port = I2C_NUM_0; ///< I2C port (default: 0)
        uint8_t i2c_addr = 0x3C;      ///< SSD1306 I2C address
    };

    struct PopupLayout
    {
        int container_width;       // total width of the popup area
        int container_height;      // total height of the popup area
        int title_bar_height;      // height reserved for the title bar
        int step_indicator_height; // height reserved for the step-indicator row
        int body_start_y;          // y-coordinate where the main body content begins

        /**
         * Compute how wide each step-indicator cell should be,
         * given the total number of steps in this workflow.
         */
        int get_step_width(uint8_t step_count) const
        {
            return container_width / step_count;
        }
    };

    /**
     * Wrapper for SSD1306-based UI using LVGL
     */
    class SSD1306
    {
    public:
        explicit SSD1306(const SSD1306Config &cfg);
        esp_err_t init();

        // Main menu entry point
        void renderMenuList(const menu::MenuState &st);
        void showMenuList(uint8_t page);
        void showPage();
        void showPopup();
        void renderMenuPage(const menu::MenuState &st);
        void renderPopup(const menu::MenuState &st);

    private:
        SSD1306Config cfg;
        lv_disp_t *disp = nullptr;
        i2c_master_bus_handle_t i2c_bus = nullptr;
        esp_lcd_panel_io_handle_t io_handle = nullptr;
        esp_lcd_panel_handle_t panel_handle = nullptr;

        // LVGL objects
        lv_obj_t *topbar_label = nullptr;
        lv_obj_t *menuContainer = nullptr;
        lv_obj_t *popupContainer = nullptr;
        lv_obj_t *pageContainer = nullptr;

        std::array<lv_obj_t *, menu::menuItemCnt> menuItems{};
        int8_t lastSelected = -1;

        // Internal helpers
        esp_err_t configureI2C();
        esp_err_t configureSSD1306();
        esp_err_t configureLVGL();
        void renderTopBar(const menu::MenuState &st, lv_obj_t *scr);

        // Menu building and selection
        void initMenuList(lv_obj_t *scr);
        void selectMenuItem(uint8_t page);

        void renderPopupHeader(const menu::PopupEntry &entry);
        void renderPopupList(const menu::MenuState &st, const PopupLayout &layout);
        void renderPopupInput(const menu::MenuState &st, const PopupLayout &layout);
        void renderPopupConfirm(const menu::MenuState &st, const PopupLayout &layout);
    };

} // namespace ui
