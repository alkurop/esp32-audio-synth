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
#include "truncate.hpp"

static const char *TAG = "ui::SSD1306";

using namespace ui;
using namespace menu;

#define MENU_TRUNCATE_LEN 3
#define PARAM_TRUNCATE_LEN 4

static constexpr int ITEM_H = 16;

void Display::initMenuList(lv_obj_t *scr)
{
    static constexpr int ITEM_H = 16;

    // 1) create & position the scrolling container
    menuContainer = lv_obj_create(scr);
    lv_obj_set_size(menuContainer, cfg.width, cfg.height - ITEM_H);
    lv_obj_align_to(menuContainer, topbar_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);
    lv_obj_set_scrollbar_mode(menuContainer, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_scroll_dir(menuContainer, LV_DIR_VER);

    // Remove internal padding if any
    lv_obj_set_style_pad_left(menuContainer, 0, 0);
    lv_obj_set_style_pad_right(menuContainer, 0, 0);

    // 2) total items = pages + popup workflows
    const uint8_t pageCnt = static_cast<uint8_t>(PAGE_COUNT);
    const uint8_t wfCnt = static_cast<uint8_t>(WORKFLOW_COUNT);
    const uint8_t itemCnt = pageCnt + wfCnt;

    // 3) build each menu line
    for (uint8_t i = 0; i < itemCnt; ++i)
    {
        const char *txt = nullptr;

        if (i < pageCnt)
        {
            txt = menuPages[i].title;
        }
        else
        {
            uint8_t wfIdx = i - pageCnt;
            if (wfIdx < static_cast<uint8_t>(Workflow::Count))
                txt = popupMenuTitles[wfIdx];
        }

        // Create the label for item title
        lv_obj_t *lbl = lv_label_create(menuContainer);
        lv_label_set_text(lbl, txt);
        lv_obj_set_width(lbl, lv_obj_get_width(menuContainer));
        lv_obj_set_y(lbl, i * ITEM_H);
        lv_obj_set_style_pad_left(lbl, 0, 0);
        lv_obj_set_style_pad_right(lbl, 0, 0);
        lv_label_set_long_mode(lbl, LV_LABEL_LONG_CLIP);
        lv_obj_set_style_text_align(lbl, LV_TEXT_ALIGN_LEFT, 0);

        // Create the index label for all items
        char indexStr[8];
        std::snprintf(indexStr, sizeof(indexStr), "%d", i + CONFIG_HUMAN_INT_OFFSET);

        lv_obj_t *indexLbl = lv_label_create(menuContainer);
        lv_label_set_text(indexLbl, indexStr);
        lv_obj_align(indexLbl, LV_ALIGN_TOP_RIGHT, -6, i * ITEM_H); // <- Offset from right
        lv_obj_set_style_text_align(indexLbl, LV_TEXT_ALIGN_RIGHT, 0);
        lv_obj_set_style_pad_right(indexLbl, 2, 0);
        lv_obj_set_style_pad_left(indexLbl, 3, 0);
        lv_obj_set_style_bg_opa(indexLbl, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);

        // Cache for later (e.g. highlighting)
        menuItems[i] = lbl;
    }
}
void Display::renderMenuList(const menu::MenuState &st)
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

void Display::showMenuList(uint8_t page)
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

void Display::renderMenuPage(const menu::MenuState &st)
{
    if (!lvgl_port_lock(0))
        return;

    lv_obj_t *scr = lv_scr_act();

    // Hide the main menu list
    showPage();

    // Compute layout
    int container_w = cfg.width - 8;
    const auto &pi = menuPages[st.menuItemIndex];
    int fieldCount = std::min<uint8_t>(pi.fieldCount, 4);
    if (fieldCount == 0)
    {
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
        if (fi.type == FieldType::Options)
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

void Display::showPage()
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

void Display::selectMenuItem(uint8_t page)
{
    // clear previous selection
    if (menuLastSelected >= 0)
    {
        lv_obj_clear_state(menuItems[menuLastSelected], LV_STATE_CHECKED);
    }
    // apply new style
    lv_obj_add_state(menuItems[page], LV_STATE_CHECKED);
    lv_obj_set_style_bg_opa(menuItems[page], LV_OPA_COVER, LV_STATE_CHECKED);
    lv_obj_set_style_bg_color(menuItems[page], lv_color_black(), LV_STATE_CHECKED);
    lv_obj_set_style_text_color(menuItems[page], lv_color_white(), LV_STATE_CHECKED);
    lv_obj_scroll_to_view(menuItems[page], LV_ANIM_OFF);
    menuLastSelected = page;
}

void Display::renderTopBar(const menu::MenuState &st, lv_obj_t *scr)
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
    std::snprintf(buf, sizeof(buf), "Voice %u  Ch %u  Vol %u", st.voiceIndex + CONFIG_HUMAN_INT_OFFSET, st.channel + CONFIG_HUMAN_INT_OFFSET, st.volume);
    lv_label_set_text(topbar_label, buf);
    lv_obj_invalidate(topbar_label);
};

void Display::showPopup()
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
