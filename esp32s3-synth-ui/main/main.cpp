#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <cstdio>

#include "rotary.hpp"
#include "button.hpp"
#include "ssd1306.hpp"
#include "menu.hpp"
#include "encoder_range.hpp"
#include "mapping.hpp"
#include "config.hpp"

using namespace ui;
using namespace menu;

SSD1306 display = SSD1306(displayConfig);

Rotary rotary0(cfg0);
Rotary rotary1(cfg1);
Rotary rotary2(cfg2);
Rotary rotary3(cfg3);
Rotary *encoders[ENCODER_COUNT] = {&rotary0, &rotary1, &rotary2, &rotary3};

Button button0;
Button button1;
Button button2;
Button button3;

Menu menuHolder(VOICE_COUNT);

auto rotaryCallback = [](uint8_t id, uint8_t newValue)
{
    menuHolder.rotateKnob(id, newValue);
};

auto left = [](uint8_t number, bool state)
{
    if (state)
    {
        menuHolder.exitPage();
        ESP_LOGI(TAG, "Button left");
    }
};
auto right = [](uint8_t number, bool state)
{
    if (state)
    {
        menuHolder.enterMenuPage();
        ESP_LOGI(TAG, "Button right");
    }
};
auto up = [](uint8_t number, bool state)
{
    if (!state)
        return;
    ESP_LOGI(TAG, "Button up");
};
auto down = [](uint8_t number, bool state)
{
    if (!state)
        return;
    ESP_LOGI(TAG, "Button down");
};

static QueueHandle_t menuRenderQueue = nullptr;
static void render_task(void *arg)
{
    auto disp = static_cast<SSD1306 *>(arg);
    MenuState st;

    ESP_LOGI(TAG, "Render task running on core %d", xPortGetCoreID());
    for (;;)
    {
        // wait forever for a new state
        if (xQueueReceive(menuRenderQueue, &st, portMAX_DELAY) == pdTRUE)
        {
            for (int i = 0; i < ENCODER_COUNT; i++)
            {
                updateEncoder(encoders[i], st.encoderRanges[i]);
            }
            // draw according to mode
            switch (st.mode)
            {
            case menu::AppMode::MenuList:
                disp->renderMenuList(st);
                break;
            case menu::AppMode::Page:
                disp->renderMenuPage(st);
                break;
            case menu::AppMode::Popup:
                disp->renderPopup(st);
                break;
            default:
                break;
            }
        }
    }
}

void createMenuRenderTask()
{
    // — Create a 1‐element queue and start render_task on core 1
    menuRenderQueue = xQueueCreate(1, sizeof(menu::MenuState));
    configASSERT(menuRenderQueue);

    BaseType_t ok = xTaskCreatePinnedToCore(
        render_task,              // task function
        "menu_render_task",       // name
        RENDER_TASK_STACK,        // stack (bytes)
        &display,                 // arg (SSD1306*)
        configMAX_PRIORITIES - 1, // priority
        nullptr,                  // no handle needed
        1                         // pin to core 1
    );
    configASSERT(ok == pdPASS);
}

auto updateCallback = [](Page page, uint8_t field, int16_t value)
{
    ESP_LOGI(TAG, "Send page %d field %d value %d", static_cast<int16_t>(page), field, value);
};

extern "C" void app_main()
{
    rotary0.init(rotaryCallback);
    rotary1.init(rotaryCallback);
    rotary2.init(rotaryCallback);
    rotary3.init(rotaryCallback);

    ESP_ERROR_CHECK(button0.init(B0, 0, up));
    ESP_ERROR_CHECK(button1.init(B1, 1, down));
    ESP_ERROR_CHECK(button2.init(B2, 2, right));
    ESP_ERROR_CHECK(button3.init(B3, 3, left));

    button0.install();
    button1.install();
    button2.install();
    button3.install();
    ESP_ERROR_CHECK(display.init());
    createMenuRenderTask();

    menuHolder.init([](const MenuState &state)
                    {
        // overwrite any pending state so we only keep the latest
        xQueueOverwrite(menuRenderQueue, &state); }, updateCallback);
}
