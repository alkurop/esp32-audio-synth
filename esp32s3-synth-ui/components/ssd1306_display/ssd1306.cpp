#include <esp_err.h>
#include <esp_log.h>
#include <esp_lcd_panel_io.h>
#include <esp_lcd_panel_ops.h>
#include <esp_lcd_panel_dev.h>
#include <esp_lcd_panel_ssd1306.h>
#include <cstdio> // for std::snprintf
#include <cstring>
#include <algorithm>
#include "ssd1306.hpp"
#include "truncate.hpp"

static const char *TAG = "ui::SSD1306";

#define LCD_CMD_BITS 8
#define LCD_PARAM_BITS 8
#define MENU_TRUNCATE_LEN 3
#define PARAM_TRUNCATE_LEN 4
#define LCD_PIXEL_CLOCK_HZ (400 * 1000)

using namespace ui;
using namespace menu;

static constexpr int ITEM_H = 16;

SSD1306::SSD1306(const SSD1306Config &cfg)
    : cfg(cfg)
{
}

esp_err_t SSD1306::init()
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

esp_err_t SSD1306::configureI2C()
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

esp_err_t SSD1306::configureSSD1306()
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

esp_err_t SSD1306::configureLVGL()
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

void SSD1306::initMenuList(lv_obj_t *scr)
{
    static constexpr int ITEM_H = 16;

    // 1) create container
    menuContainer = lv_obj_create(scr);
    lv_obj_set_size(menuContainer, cfg.width, cfg.height - 16);
    lv_obj_align_to(menuContainer, topbar_label,
                    LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);
    lv_obj_set_scrollbar_mode(menuContainer, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_scroll_dir(menuContainer, LV_DIR_VER);

    // 2) total items = pages + popup workflows
    const auto itemCnt = static_cast<uint8_t>(menu::menuItemCnt);
    const auto pageCnt = static_cast<uint8_t>(menu::pageCnt);

    for (uint8_t i = 0; i < itemCnt; ++i)
    {
        // decide whether this slot is a real page or a popup workflow
        const char *txt;
        if (i < pageCnt)
        {
            txt = menu::menuPages[i].title;
        }
        else
        {
            auto wf = menu::popupWorkflows[i - pageCnt];
            txt = wf.title;
        }

        // create & position label
        lv_obj_t *lbl = lv_label_create(menuContainer);
        lv_label_set_text(lbl, txt);
        lv_obj_set_width(lbl, lv_obj_get_width(menuContainer));
        lv_obj_set_y(lbl, i * ITEM_H);

        // remember it for highlighting later
        menuItems[i] = lbl;
    }
}

void SSD1306::renderMenuList(const menu::MenuState &st)
{
    if (!lvgl_port_lock(0))
        return;
    lv_obj_t *scr = lv_scr_act();
    // Update title bar
    renderTopBar(st, scr);
    // One-time build of the menu
    if (!menuContainer)
    {
        initMenuList(scr);
    }

    // Show menu and highlight
    showMenuList(st.menuItemIndex);
    lv_timer_handler();
    lvgl_port_unlock();
}

void SSD1306::showMenuList(uint8_t page)
{
    if (menuContainer)
    {
        lv_obj_clear_flag(menuContainer, LV_OBJ_FLAG_HIDDEN);
    }
    if (pageContainer)
    {
        lv_obj_add_flag(pageContainer, LV_OBJ_FLAG_HIDDEN);
    }

    if (popupContainer)
    {
        lv_obj_add_flag(popupContainer, LV_OBJ_FLAG_HIDDEN);
    }
    selectMenuItem(page);
}

void SSD1306::renderMenuPage(const menu::MenuState &st)
{
    // Lock LVGL
    if (!lvgl_port_lock(0))
        return;
    lv_obj_t *scr = lv_scr_act();

    // Hide the main menu list
    showPage();

    // Compute layout
    int container_w = cfg.width - 8;
    const auto &pi = menu::menuPages[st.menuItemIndex];
    int fieldCount = std::min<uint8_t>(pi.fieldCount, 4);
    if (fieldCount == 0)
    {
        lvgl_port_unlock();
        return;
    }
    int cell_width = container_w / fieldCount;
    int labelY = 0;
    int valueY = 18;
    int total_w = cell_width * fieldCount;
    int start_x = (container_w - total_w) / 2;

    // One-time container under top bar
    if (!pageContainer)
    {
        pageContainer = lv_obj_create(scr);
        lv_obj_set_size(pageContainer, container_w, cfg.height - 18);
        lv_obj_align_to(pageContainer, topbar_label,
                        LV_ALIGN_OUT_BOTTOM_LEFT, 0, 4);
    }
    lv_obj_clean(pageContainer);

    // Draw each field
    for (int k = 0; k < fieldCount; ++k)
    {
        const auto &fi = pi.fields[k];

        // — parameter label, truncated —
        char lblBuf[MENU_TRUNCATE_LEN + 1];
        ui::truncateForMenu(fi.label, lblBuf, MENU_TRUNCATE_LEN);
        lv_obj_t *lbl = lv_label_create(pageContainer);
        lv_obj_set_style_text_font(lbl, &lv_font_montserrat_12, LV_STATE_DEFAULT);
        lv_label_set_text(lbl, lblBuf);
        size_t len = strlen(lblBuf);
        lv_font_glyph_dsc_t g_lbl;
        lv_font_get_glyph_dsc(&lv_font_montserrat_12, &g_lbl, (uint32_t)lblBuf[0], 0);
        int lbl_w = g_lbl.box_w * (int)len;
        lv_obj_set_pos(lbl, start_x + k * cell_width + (cell_width - lbl_w) / 2, labelY);

        // — value or option —
        lv_obj_t *val = lv_label_create(pageContainer);
        if (fi.type == menu::FieldType::Options)
        {
            char optBuf[PARAM_TRUNCATE_LEN + 1];
            const char *optStr = fi.opts[st.fieldValues[k]];
            ui::truncateForMenu(optStr, optBuf, PARAM_TRUNCATE_LEN);
            lv_obj_set_style_text_font(val, &lv_font_montserrat_12, LV_STATE_DEFAULT);
            lv_label_set_text(val, optBuf);
            size_t olen = strlen(optBuf);
            lv_font_glyph_dsc_t g_opt;
            lv_font_get_glyph_dsc(&lv_font_montserrat_12, &g_opt, (uint32_t)optBuf[0], 0);
            int opt_w = g_opt.box_w * (int)olen;
            lv_obj_set_pos(val, start_x + k * cell_width + (cell_width - opt_w) / 2, valueY);
        }
        else
        {
            char buf[4];
            std::snprintf(buf, sizeof(buf), "%02u", st.fieldValues[k]);
            lv_obj_set_style_text_font(val, &lv_font_montserrat_14, LV_STATE_DEFAULT);
            lv_label_set_text(val, buf);
            lv_font_glyph_dsc_t g_val;
            lv_font_get_glyph_dsc(&lv_font_montserrat_14, &g_val, (uint32_t)'0', 0);
            int dig_w = g_val.box_w;
            lv_obj_set_pos(val, start_x + k * cell_width + (cell_width - dig_w * 2) / 2, valueY);
        }
    }

    // Refresh & unlock
    lv_timer_handler();
    lvgl_port_unlock();
}

void SSD1306::showPage()
{
    if (menuContainer)
    {
        lv_obj_add_flag(menuContainer, LV_OBJ_FLAG_HIDDEN);
    }
    if (pageContainer)
    {
        lv_obj_clear_flag(pageContainer, LV_OBJ_FLAG_HIDDEN);
    }
    if (popupContainer)
    {
        lv_obj_add_flag(popupContainer, LV_OBJ_FLAG_HIDDEN);
    }
}

void SSD1306::selectMenuItem(uint8_t page)
{
    // clear previous selection
    if (lastSelected >= 0)
    {
        lv_obj_clear_state(menuItems[lastSelected], LV_STATE_CHECKED);
    }
    // apply new style
    lv_obj_add_state(menuItems[page], LV_STATE_CHECKED);
    lv_obj_set_style_bg_opa(menuItems[page], LV_OPA_COVER, LV_STATE_CHECKED);
    lv_obj_set_style_bg_color(menuItems[page], lv_color_black(), LV_STATE_CHECKED);
    lv_obj_set_style_text_color(menuItems[page], lv_color_white(), LV_STATE_CHECKED);
    lv_obj_scroll_to_view(menuItems[page], LV_ANIM_OFF);
    lastSelected = page;
}

void SSD1306::renderTopBar(const menu::MenuState &st, lv_obj_t *scr)
{
    if (!topbar_label)
    {
        topbar_label = lv_label_create(scr);

        lv_obj_set_style_text_font(topbar_label, &lv_font_montserrat_12, 0);

        // 2) remove any top/bottom padding
        lv_obj_set_style_pad_top(topbar_label, 0, 0);
        lv_obj_set_style_pad_bottom(topbar_label, 0, 0);

        // 3) remove extra line spacing (if your LVGL version supports it)
        lv_obj_set_style_text_line_space(topbar_label, 0, 0);

        // now align flush to the top
        lv_obj_align(topbar_label, LV_ALIGN_TOP_LEFT, 4, 0);
    }

    char buf[32];
    std::snprintf(buf, sizeof(buf), "Voice %u  Ch %u  Vol %u", st.voice + CONFIG_HUMAN_INT_OFFSET, st.channel + CONFIG_HUMAN_INT_OFFSET, st.volume);
    lv_label_set_text(topbar_label, buf);
    lv_obj_invalidate(topbar_label);
};

void SSD1306::renderPopup(const menu::MenuState &st)
{
    // 1) Lock LVGL
    if (!lvgl_port_lock(0))
        return;
    lv_obj_t *scr = lv_scr_act();

    // 2) Figure out which workflow we’re in (same as before)…
    int wfIndex = -1;
    for (int i = 0; i < workflowCnt; ++i)
    {
        if (popupWorkflows[i].baseMode == st.popup.mode)
        {
            wfIndex = i;
            break;
        }
    }
    if (wfIndex < 0)
    {
        lvgl_port_unlock();
        return;
    }
    const PopupWorkflow &wf = popupWorkflows[wfIndex];

    // ───────────────────────────
    // 2.5) Update the top bar text with the popup title
    lv_label_set_text(topbar_label, wf.title);
    lv_obj_align(topbar_label, LV_ALIGN_TOP_MID, 0, 0);
    // ───────────────────────────

    // 3) Show/hide as needed
    showPopup(); // hide main list, show popup container

    // 4) Compute step offset within the workflow
    int stepOffset = static_cast<int>(st.popup.mode) - static_cast<int>(wf.baseMode);
    stepOffset = std::clamp(stepOffset, 0, int(wf.count) - 1);

    // 5) Layout
    int container_w = cfg.width - 8;
    int container_h = cfg.height - 18;
    int title_h = 12;
    int step_h = 10;
    int body_y = title_h + step_h + 4;

    if (!popupContainer)
    {
        popupContainer = lv_obj_create(scr);
        lv_obj_set_size(popupContainer, container_w, container_h);
        lv_obj_align_to(popupContainer, topbar_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 4);
    }
    lv_obj_clean(popupContainer);

    // 6) Draw workflow title
    {
        lv_obj_t *lbl = lv_label_create(popupContainer);
        lv_obj_set_style_text_font(lbl, &lv_font_montserrat_14, LV_STATE_DEFAULT);
        lv_label_set_text(lbl, wf.title);
        lv_obj_align(lbl, LV_ALIGN_TOP_MID, 0, 0);
    }

    // 7) Draw step labels across top
    int step_w = container_w / wf.count;
    for (uint8_t i = 0; i < wf.count; ++i)
    {
        lv_obj_t *s = lv_label_create(popupContainer);
        lv_obj_set_style_text_font(s, &lv_font_montserrat_12, LV_STATE_DEFAULT);
        lv_label_set_text(s, wf.steps[i]);
        lv_obj_set_pos(s, i * step_w + (step_w - lv_obj_get_width(s)) / 2, title_h);

        if (i == stepOffset)
        {
            lv_obj_t *bar = lv_obj_create(popupContainer);
            lv_obj_set_size(bar, step_w - 4, 2);
            lv_obj_set_style_bg_color(bar, lv_color_white(), LV_STATE_DEFAULT);
            lv_obj_set_pos(bar, i * step_w + 2, title_h + step_h - 2);
        }
    }

    // 8) If this is a “rename” step, draw the 4-char buffer + cursor
    if (wf.steps[stepOffset][0] == 'R') // crude test: “Rename …”
    {
        const auto &p = st.popup;
        char buf[6] = {};
        std::memcpy(buf, p.editName, 4);
        for (int i = 0; i < 4; ++i)
            if (!buf[i])
                buf[i] = '_';

        int cell = container_w / 4;
        for (int i = 0; i < 4; ++i)
        {
            lv_obj_t *c = lv_label_create(popupContainer);
            lv_obj_set_style_text_font(c, &lv_font_montserrat_14, LV_STATE_DEFAULT);
            char ch[2] = {buf[i], '\0'};
            lv_label_set_text(c, ch);
            int x = i * cell + (cell - 8) / 2;
            lv_obj_set_pos(c, x, body_y);

            if (i == st.popup.slotIndex)
            {
                lv_obj_t *cur = lv_obj_create(popupContainer);
                lv_obj_set_size(cur, 8, 2);
                lv_obj_set_style_bg_color(cur, lv_color_white(), LV_STATE_DEFAULT);
                lv_obj_set_pos(cur, x, body_y + 14);
            }
        }
    }

    // 9) Done! refresh & unlock
    lv_timer_handler();
    lvgl_port_unlock();
}

void SSD1306::showPopup()
{
    if (menuContainer)
    {
        lv_obj_add_flag(menuContainer, LV_OBJ_FLAG_HIDDEN);
    }
    if (pageContainer)
    {
        lv_obj_add_flag(pageContainer, LV_OBJ_FLAG_HIDDEN);
    }
    if (popupContainer)
    {
        lv_obj_clear_flag(popupContainer, LV_OBJ_FLAG_HIDDEN);
    }
}
