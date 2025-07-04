#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "menu.hpp"
#include "config.hpp"
#include "esp_log.h"
#include "display.hpp"
#include "rotary.hpp"

using namespace menu;
using namespace ui;

struct RenderTaskCtx
{
    Display *display;
    Rotary **encoders; // pointer to first element
    size_t encoderCount; // pointer to first element
};

static QueueHandle_t menuRenderQueue = nullptr;
static void render_task(void *arg)
{
    auto context = static_cast<RenderTaskCtx *>(arg);
    auto disp = context->display;
    auto encoders = context->encoders;
    auto encoderCount = context->encoderCount;
    MenuState st;

    ESP_LOGI(TAG, "Render task running on core %d", xPortGetCoreID());
    for (;;)
    {
        // wait forever for a new state
        if (xQueueReceive(menuRenderQueue, &st, portMAX_DELAY) == pdTRUE)
        {
            for (int i = 0; i < encoderCount; i++)
            {
                updateEncoder(encoders[i], st.encoderRanges[i]);
            }
            // draw according to mode
            switch (st.mode)
            {
            case AppMode::MenuList:
                disp->renderMenuList(st);
                break;
            case AppMode::Page:
                disp->renderMenuPage(st);
                break;
            case AppMode::Popup:
                disp->renderPopup(st);
                break;
            case AppMode::Loading:
                disp->renderLoading();
            default:
                break;
            }
        }
    }
}

static void createMenuRenderTask(RenderTaskCtx *context)
{
    // — Create a 1‐element queue and start render_task on core 1
    menuRenderQueue = xQueueCreate(1, sizeof(menu::MenuState));
    configASSERT(menuRenderQueue);


    BaseType_t ok = xTaskCreate(
        render_task,              // task function
        "menu_render_task",       // name
        8 * 1024,                 // stack (bytes)
        context,                  // arg (SSD1306*)
        configMAX_PRIORITIES - 5, // priority
        nullptr                  // no handle needed
    );
    configASSERT(ok == pdPASS);
}
