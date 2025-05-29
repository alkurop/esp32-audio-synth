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
static constexpr int ITEM_HEIGHT = 14;

// In display_popup.cpp:

void SSD1306::renderPopup(const menu::MenuState &st)
{
    // 0) Try to lock LVGL
    if (!lvgl_port_lock(0))
        return;

    // 1) Grab workflow index and validate it
    const auto &ps = st.popup;
    size_t wfIdx = static_cast<size_t>(ps.workflowIndex);
    constexpr size_t wfCount = static_cast<size_t>(menu::Workflow::Count);
    if (wfIdx >= wfCount)
    {
        lvgl_port_unlock();
        return;
    }

    // 2) Lookup workflow and step entry
    const auto &wf = menu::popupWorkflows[wfIdx];
    const auto &entry = wf.steps[ps.stepIndex];

    // 3) Render the common header
    renderPopupHeader(entry);

    // 4) Compute a concrete layout
    PopupLayout layout{
        /*container_width*/ cfg.width - 8,
        /*container_height*/ cfg.height - 18,
        /*title_bar_height*/ 12,
        /*step_indicator_height*/ 10,
        /*body_start_y*/ (12 + 10 + 4)};

    // 5) Create the container on first use
    if (!popupContainer)
    {
        popupContainer = lv_obj_create(lv_scr_act());
        lv_obj_set_size(popupContainer,
                        layout.container_width,
                        layout.container_height);
        lv_obj_align_to(popupContainer,
                        topbar_label,
                        LV_ALIGN_OUT_BOTTOM_LEFT,
                        0, 4);
        lastWorkflowIdx = static_cast<size_t>(-1);
        lastStepIdx = -1;
    }

    // 6) Decide if we’ve jumped workflow or stepped in/out of a list
    bool sameWorkflow = (wfIdx == lastWorkflowIdx);
    bool sameStep = (ps.stepIndex == lastStepIdx);

    bool wasList = false;
    if (sameWorkflow && lastStepIdx >= 0)
    {
        auto &oldStep = menu::popupWorkflows[lastWorkflowIdx]
                            .steps[lastStepIdx];
        wasList = isListPopup(oldStep.mode);
    }

    bool isListNow = isListPopup(entry.mode);

    // 7) Clear & rebuild only when needed
    if (!sameWorkflow              // brand‐new workflow
        || !sameStep               // moved step in the same workflow
        || (wasList != isListNow)) // crossed into or out of a list popup
    {
        lv_obj_clean(popupContainer);

        if (isListNow)
        {
            // Build the list once
            initPopupList(st, layout);
        }
    }
    else if (isListNow)
    {
        // still in the same list popup: just move the highlight
        selectPopupItem(ps.slotIndex);
    }

    // 8) If it’s *not* a list popup, draw the other UIs
    if (!isListNow)
    {
        if (isInputPopup(entry.mode))
        {
            renderPopupInput(st, layout);
        }
        else
        {
            renderPopupConfirm(st, layout);
        }
    }

    // 9) Remember for next frame
    lastWorkflowIdx = wfIdx;
    lastStepIdx = ps.stepIndex;

    // 10) Finish up
    showPopup();
    lv_timer_handler();
    lvgl_port_unlock();
}

void SSD1306::renderPopupHeader(const menu::PopupEntry &entry)
{
    lv_label_set_text(topbar_label, entry.title);
    lv_obj_align(topbar_label, LV_ALIGN_TOP_LEFT, 0, 0);
}

void SSD1306::renderPopupInput(const menu::MenuState &st, const PopupLayout &L)
{
    const auto &ps = st.popup;
    char buf[5] = {};

    // — Prefill logic —
    if (ps.editName[0] != '\0')
    {
        // You’ve already started editing: use what’s in editName
        std::memcpy(buf, ps.editName, 4);
    }
    else
    {
        // First time in: copy the existing name from the list
        const auto &entry = ps.listItems[ps.slotIndex];
        // First, make sure buf is zeroed:
        char buf[5] = {0};

        // Copy up to 4 chars from the std::string into your C-string:
        std::strncpy(buf, entry.name.c_str(), 4);

        // Guarantee null-termination:
        buf[4] = '\0';
    }
    buf[4] = '\0'; // ensure termination

    // Replace any '\0' with underscore for display
    for (int i = 0; i < 4; ++i)
    {
        if (buf[i] == '\0')
            buf[i] = '_';
    }

    // layout: split container into 4 cells
    int cell = L.container_width / 4;
    for (int i = 0; i < 4; ++i)
    {
        // character label
        lv_obj_t *c = lv_label_create(popupContainer);
        lv_obj_set_style_text_font(c, &lv_font_montserrat_14, LV_STATE_DEFAULT);
        char ch[2] = {buf[i], '\0'};
        lv_label_set_text(c, ch);

        int x = i * cell + (cell - 8) / 2;
        lv_obj_set_pos(c, x, L.body_start_y);

        // underline the “cursor” position
        if (i == ps.stepIndex)
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
    snprintf(buf, sizeof(buf), "%s", name);

    // Create one centered label
    lv_obj_t *lbl = lv_label_create(popupContainer);
    lv_label_set_text(lbl, buf);
    lv_obj_align(lbl, LV_ALIGN_CENTER, 0, 0);
}

/// On each frame/tick, either init or just select
void SSD1306::renderPopupList(const menu::MenuState &st, const PopupLayout &L)
{
    int16_t cur = st.popup.slotIndex;

    // if we haven’t built for this popup, or jumped to a different list entirely:
    if (listPopupItemLastSelected < 0 ||
        /* you could also detect a different popup mode here if needed */
        cur < 0 ||
        cur >= (int)popupLabels.size())
    {
        initPopupList(st, L);
    }
    // otherwise just update the highlight
    else if (cur != listPopupItemLastSelected)
    {
        selectPopupItem(cur);
    }
    // if cur == popupLastSelected, do nothing
}

 void SSD1306::initPopupList(const menu::MenuState &st, const PopupLayout &L)
{
    // 1) wipe out any old children
    lv_obj_clean(popupContainer);

    // 2) rebuild the labels & icons
    popupLabels.clear();
    // (optional) popupIcons.clear(); if you want to track the icons too
    popupLabels.reserve(st.popup.listItems.size());

    for (size_t i = 0; i < st.popup.listItems.size(); ++i)
    {
        const auto &item = st.popup.listItems[i];
        int y = int(i) * ITEM_HEIGHT;

        // 2a) create the name label
        auto *lbl = lv_label_create(popupContainer);
        lv_label_set_text(lbl, item.name.c_str());
        lv_obj_set_width(lbl, L.container_width - 16); // leave space for the icon
        lv_obj_set_pos(lbl, 0, y);
        popupLabels.push_back(lbl);

        // 2b) create the "loaded" indicator
        auto *icon = lv_label_create(popupContainer);
        lv_label_set_text(icon, "X");       // or "*" or even an image
        lv_obj_set_style_text_font(icon, &lv_font_montserrat_14, 0);
        // position it right‐aligned in that row:
        lv_obj_align_to(icon, lbl, LV_ALIGN_OUT_RIGHT_MID, 4, 0);

        // show or hide it
        if (!item.loaded) {
            lv_obj_add_flag(icon, LV_OBJ_FLAG_HIDDEN);
        }
        // (optional) store icons if you need to update them later:
        // popupIcons.push_back(icon);
    }

    // 3) reset selection so selectPopupItem will redraw it
    listPopupItemLastSelected = -1;

    // 4) highlight & scroll into view
    selectPopupItem(st.popup.slotIndex);
}

/// Clear the old “checked” state and apply it to the new index
void SSD1306::selectPopupItem(int16_t newIndex)
{
    auto size = popupLabels.size();
    // // clear previous
    if (listPopupItemLastSelected >= 0 && listPopupItemLastSelected < size)
    {
        lv_obj_clear_state(popupLabels[listPopupItemLastSelected], LV_STATE_CHECKED);
    }

    // set new
    if (newIndex >= 0 &&
        newIndex < size)
    {
        auto *lbl = popupLabels[newIndex];
        lv_obj_add_state(lbl, LV_STATE_CHECKED);
        lv_obj_set_style_bg_opa(lbl, LV_OPA_COVER, LV_STATE_CHECKED);
        lv_obj_set_style_bg_color(lbl, lv_color_black(), LV_STATE_CHECKED);
        lv_obj_set_style_text_color(lbl, lv_color_white(), LV_STATE_CHECKED);
        lv_obj_scroll_to_view(lbl, LV_ANIM_OFF);
    }

    listPopupItemLastSelected = newIndex;
}
