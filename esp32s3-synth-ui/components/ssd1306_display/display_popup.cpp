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

using namespace ui;
using namespace menu;

#define MENU_TRUNCATE_LEN 3
#define PARAM_TRUNCATE_LEN 4

void SSD1306::renderPopup(const menu::MenuState &st)
{
    using namespace menu;
    const auto &ps = st.popup;

    // 1) Validate workflow index
    auto wfIdx = static_cast<size_t>(ps.workflowIndex);
    if (wfIdx >= static_cast<size_t>(Workflow::Count))
        return;

    // 2) Grab the workflow & current step entry
    const auto &wf = popupWorkflows[wfIdx];
    const auto &entry = wf.steps[ps.stepIndex];

    // 3) Update top‐bar with the step’s title
    lv_label_set_text(topbar_label, entry.title);
    lv_obj_align(topbar_label, LV_ALIGN_TOP_MID, 0, 0);

    // 4) Show the popup container (hides main menu)
    showPopup();

    // 5) Layout constants
    int container_w = cfg.width - 8;
    int container_h = cfg.height - 18;
    int title_h = 12;
    int step_h = 10;
    int body_y = title_h + step_h + 4;

    // 6) (Re)create & clear the container
    if (!popupContainer)
    {
        popupContainer = lv_obj_create(lv_scr_act());
        lv_obj_set_size(popupContainer, container_w, container_h);
        lv_obj_align_to(popupContainer, topbar_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 4);
    }
    lv_obj_clean(popupContainer);

    // 7) Draw the step‐indicator bar & labels
    int step_w = container_w / wf.stepCount;
    for (size_t i = 0; i < wf.stepCount; ++i)
    {
        // step label
        lv_obj_t *lbl = lv_label_create(popupContainer);
        lv_obj_set_style_text_font(lbl, &lv_font_montserrat_12, LV_STATE_DEFAULT);
        lv_label_set_text(lbl, wf.steps[i].title);
        lv_obj_set_pos(lbl,
                       i * step_w + (step_w - lv_obj_get_width(lbl)) / 2,
                       title_h);

        // highlight current step
        if (i == ps.stepIndex)
        {
            lv_obj_t *bar = lv_obj_create(popupContainer);
            lv_obj_set_size(bar, step_w - 4, 2);
            lv_obj_set_style_bg_color(bar, lv_color_white(), LV_STATE_DEFAULT);
            lv_obj_set_pos(bar, i * step_w + 2, title_h + step_h - 2);
        }
    }

    // 8) Render the step’s content
    switch (entry.mode)
    {
    case PopupMode::SaveVoiceRename:
    case PopupMode::SaveProjectRename:
    {
        // draw the 4-char edit buffer + cursor
        char buf[5] = {};
        std::memcpy(buf, ps.editName, 4);
        for (int i = 0; i < 4; ++i)
            if (buf[i] == '\0')
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

            // cursor under current slotIndex
            if (i == ps.slotIndex)
            {
                lv_obj_t *cur = lv_obj_create(popupContainer);
                lv_obj_set_size(cur, 8, 2);
                lv_obj_set_style_bg_color(cur, lv_color_white(), LV_STATE_DEFAULT);
                lv_obj_set_pos(cur, x, body_y + 14);
            }
        }
        break;
    }

    default:
    {
        // generic content: e.g. show slotIndex or a checkmark
        // you can replace this with your actual drawing:
        lv_obj_t *info = lv_label_create(popupContainer);
        lv_label_set_text_fmt(info, "Slot %d", ps.slotIndex + 1);
        lv_obj_align(info, LV_ALIGN_CENTER, 0, 0);
        break;
    }
    }

    // 9) Trigger LVGL refresh
    lv_timer_handler();
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
