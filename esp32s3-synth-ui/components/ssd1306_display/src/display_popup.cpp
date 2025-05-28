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
    const auto &ps = st.popup;
    size_t wfIdx = static_cast<size_t>(ps.workflowIndex);
    if (wfIdx >= static_cast<size_t>(Workflow::Count))
        return;

    // 1) Lookup workflow & entry
    const auto &wf = popupWorkflows[wfIdx];
    const auto &entry = wf.steps[ps.stepIndex];

    // 2) Header
    renderPopupHeader(entry);

    // 3) Show container (hides main menu)
    showPopup();

    // 4) Compute layout once
    PopupLayout layout{
        /*container_width*/ cfg.width - 8,
        /*container_height*/ cfg.height - 18,
        /*title_bar_height*/ 12,
        /*step_indicator_height*/ 10,
        /*body_start_y*/ 12 + 10 + 4};

    // 5) (Re)create & clear container
    if (!popupContainer)
    {
        popupContainer = lv_obj_create(lv_scr_act());
        lv_obj_set_size(popupContainer,
                        layout.container_width,
                        layout.container_height);
        lv_obj_align_to(popupContainer, topbar_label,
                        LV_ALIGN_OUT_BOTTOM_LEFT, 0, 4);
    }
    lv_obj_clean(popupContainer);

    // 6) Steps row
    renderPopupSteps(wf, ps.stepIndex, layout);

    // 7) Body by type
    if (isListPopup(entry.mode))
        renderPopupList(st, layout);
    else if (isInputPopup(entry.mode))
        renderPopupInput(st, layout);
    else
        renderPopupConfirm(st, layout);

    // 8) Trigger LVGL draw
    lv_timer_handler();
}

void SSD1306::renderPopupHeader(const menu::PopupEntry &entry)
{
    lv_label_set_text(topbar_label, entry.title);
    lv_obj_align(topbar_label, LV_ALIGN_TOP_MID, 0, 0);
}

void SSD1306::renderPopupSteps(const menu::PopupWorkflow &wf, uint8_t stepIndex, const PopupLayout &L)
{
    int step_w = L.get_step_width(wf.stepCount);
    for (uint8_t i = 0; i < wf.stepCount; ++i)
    {
        lv_obj_t *lbl = lv_label_create(popupContainer);
        lv_obj_set_style_text_font(lbl, &lv_font_montserrat_12, LV_STATE_DEFAULT);
        lv_label_set_text(lbl, wf.steps[i].title);
        lv_obj_set_pos(lbl, i * step_w + (step_w - lv_obj_get_width(lbl)) / 2, L.title_bar_height);

        if (i == stepIndex)
        {
            lv_obj_t *bar = lv_obj_create(popupContainer);
            lv_obj_set_size(bar, step_w - 4, 2);
            lv_obj_set_style_bg_color(bar, lv_color_white(), LV_STATE_DEFAULT);
            lv_obj_set_pos(bar, i * step_w + 2, L.title_bar_height + L.step_indicator_height - 2);
        }
    }
}

void SSD1306::renderPopupList(const menu::MenuState &st, const PopupLayout &L)
{
    const auto &ps = st.popup;
    const auto &wf = popupWorkflows[static_cast<size_t>(ps.workflowIndex)];
    const auto &entry = wf.steps[ps.stepIndex];

    // Choose voice vs. project names
    const auto &names = st.popup.listItems;

    // Draw up to 4 items starting at L.body_start_y
    for (size_t i = 0; i < names.size() && i < 4; ++i)
    {
        lv_obj_t *lbl = lv_label_create(popupContainer);
        lv_label_set_text(lbl, names[i].c_str());
        lv_obj_set_pos(lbl, 4, L.body_start_y + i * 14);

        if (i == ps.slotIndex)
        {
            // invert colors for the selected item
            lv_obj_set_style_text_color(lbl, lv_color_white(), LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(lbl, lv_color_black(), LV_STATE_DEFAULT);
        }
    }
}

void SSD1306::renderPopupInput(const menu::MenuState &st, const PopupLayout &L)
{
    const auto &ps = st.popup;
    char buf[5] = {};
    std::memcpy(buf, ps.editName, 4);
    for (int i = 0; i < 4; ++i)
        if (buf[i] == '\0')
            buf[i] = '_';

    int cell = L.container_width / 4;
    for (int i = 0; i < 4; ++i)
    {
        lv_obj_t *c = lv_label_create(popupContainer);
        lv_obj_set_style_text_font(c, &lv_font_montserrat_14, LV_STATE_DEFAULT);
        char ch[2] = {buf[i], '\0'};
        lv_label_set_text(c, ch);

        int x = i * cell + (cell - 8) / 2;
        lv_obj_set_pos(c, x, L.body_start_y);

        if (i == ps.slotIndex)
        {
            lv_obj_t *cur = lv_obj_create(popupContainer);
            lv_obj_set_size(cur, 8, 2);
            lv_obj_set_style_bg_color(cur, lv_color_white(), LV_STATE_DEFAULT);
            lv_obj_set_pos(cur, x, L.body_start_y + 14);
        }
    }
}

void SSD1306::renderPopupConfirm(const menu::MenuState &st, const PopupLayout &L)
{
    // simple “Done” confirmation, centered
    lv_obj_t *lbl = lv_label_create(popupContainer);
    lv_label_set_text(lbl, "Done!");
    lv_obj_set_pos(lbl, (L.container_width - lv_obj_get_width(lbl)) / 2, L.body_start_y + (L.container_height - L.body_start_y) / 2);
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
