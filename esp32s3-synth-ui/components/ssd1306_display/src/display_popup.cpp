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
    if (!lvgl_port_lock(0))
        return;

    const auto &ps = st.popup;
    size_t wfIdx = static_cast<size_t>(ps.workflowIndex);
    if (wfIdx >= static_cast<size_t>(Workflow::Count))
        return;

    // 1) Lookup workflow & entry
    const auto &wf = popupWorkflows[wfIdx];
    const auto &entry = wf.steps[ps.stepIndex];

    // 2) Header
    renderPopupHeader(entry);

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
    else
    {
        lv_obj_clean(popupContainer);
    }

    // 6) Steps row
    // renderPopupSteps(wf, ps.stepIndex, layout);

    // 7) Body by type
    if (isListPopup(entry.mode))
        renderPopupList(st, layout);
    else if (isInputPopup(entry.mode))
        renderPopupInput(st, layout);
    else
        renderPopupConfirm(st, layout);

    showPopup();
    lv_timer_handler();
    lvgl_port_unlock();
}

void SSD1306::renderPopupHeader(const menu::PopupEntry &entry)
{
    lv_label_set_text(topbar_label, entry.title);
    lv_obj_align(topbar_label, LV_ALIGN_TOP_LEFT, 0, 0);
}

void SSD1306::renderPopupList(const menu::MenuState &st, const PopupLayout &L)
{
    const auto &ps = st.popup;
    const auto &entries = ps.listItems;

    // 2) Make sure this container scrolls vertically
    lv_obj_set_scrollbar_mode(popupContainer, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_scroll_dir(popupContainer, LV_DIR_VER);

    // 3) Create one label per entry, and style the selected one
    static constexpr int ITEM_HEIGHT = 14;
    lv_obj_t *selected_lbl = nullptr;

    for (size_t i = 0; i < entries.size(); ++i)
    {
        lv_obj_t *lbl = lv_label_create(popupContainer);
        lv_label_set_text(lbl, entries[i].name.c_str());
        lv_obj_set_width(lbl, L.container_width);
        lv_obj_set_y(lbl, int(i) * ITEM_HEIGHT);

        if (i == ps.slotIndex)
        {
            // apply the “checked” style
            lv_obj_add_state(lbl, LV_STATE_CHECKED);
            lv_obj_set_style_bg_opa(lbl, LV_OPA_COVER, LV_STATE_CHECKED);
            lv_obj_set_style_bg_color(lbl, lv_color_black(), LV_STATE_CHECKED);
            lv_obj_set_style_text_color(lbl, lv_color_white(), LV_STATE_CHECKED);
            selected_lbl = lbl;
        }
    }

    // 4) Scroll the selected label into view (if any)
    if (selected_lbl)
    {
        lv_obj_scroll_to_view(selected_lbl, LV_ANIM_OFF);
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
    // Clear out any previous content
    lv_obj_clean(popupContainer);

    // Pull the edited name (4-char + ‘\0’) directly:
    const char *name = st.popup.editName;

    // Build your confirmation string
    char buf[32];
    snprintf(buf, sizeof(buf), "%s  loaded", name);

    // Create one centered label
    lv_obj_t *lbl = lv_label_create(popupContainer);
    lv_label_set_text(lbl, buf);
    lv_obj_align(lbl, LV_ALIGN_CENTER, 0, 0);
}
