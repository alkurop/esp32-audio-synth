#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <esp_err.h>
#include <cstdio>
#include <array>
#include <functional> // for std::reference_wrapper, std::ref

#include "rotary.hpp"
#include "button.hpp"
#include "display.hpp"
#include "menu.hpp"
#include "encoder_range.hpp"
#include "rotary_mapping.hpp"
#include "config.hpp"
#include "sender.hpp"
#include "midi_module.hpp"

using namespace ui;
using namespace menu;
using namespace protocol;
using namespace midi_module;

Display display = Display(displayConfig);
MidiParser midiParser;
MidiModule midiModule;

Rotary rotary0(cfg0);
Rotary rotary1(cfg1);
Rotary rotary2(cfg2);
Rotary rotary3(cfg3);
Rotary *encoders[ENCODER_COUNT] = {&rotary0, &rotary1, &rotary2, &rotary3};

Button button0;
Button button1;
Button button2;
Button button3;
Sender sender(senderConfig);

Menu menuHolder(protocol::NUM_VOICES);

// MIDI packet callback
MidiReadCallback midiReadCallback = [](const uint8_t packet[4])
{
    midiParser.feed(packet);
};

// MIDI event callbacks
MidiControllerCallback controllerCallback = [](const ControllerChange &cc)
{
    // handle CC as needed
};

MidiNoteCallback noteMessageCallback = [](const MidiNoteEvent &note)
{
    EventList events;
    events.reserve(1);
    protocol::Event e;
    e.type = protocol::EventType::MidiNote;
    e.note = note;
    events.push_back(e);
    auto result = sender.send(events);
    if (result != ESP_OK)
    {
        ESP_LOGE(TAG, "sending falied with code %d", result);
    }
};

MidiSongPositionCallback songPositionCallback = [](const SongPosition &sp)
{ ESP_LOGI(TAG, "Song Position: %d", sp.position); };

MidiTransportCallback transportCallback = [](const TransportEvent &ev)
{
    ESP_LOGI(TAG, "transport Position: %d", static_cast<int>(ev.command));
    // settingSwitch.setTransportState(ev.command);
};

BpmCounter::BpmCallback bpmCallback = [](uint8_t bpm)
{
    ESP_LOGI(TAG, "bpm: %d", bpm);
    // settingSwitch.setBpmFromMidi(bpm);
    // todo
};

auto rotaryCallback = [](uint8_t id, int16_t newValue)
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
    auto disp = static_cast<Display *>(arg);
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

auto updateCallback = [](const FieldUpdateList &updates)
{
    // 1) Wrap the updates in our tagged Event
    protocol::Event e;
    e.type = protocol::EventType::FieldUpdate;
    e.fields = updates;

    // 2) Make a one‐element EventList
    EventList events;
    events.reserve(1);
    events.push_back(std::move(e));

    // 3) Send it
    auto result = sender.send(events);
    if (result != ESP_OK)
    {
        ESP_LOGE(TAG, "sending falied with code %d", result);
    }
};

void initMidi()
{
    // // // Initialize modules
    midiModule.init(midiReadCallback);

    // Set MIDI parser callbacks
    midiParser.setControllerCallback(controllerCallback);
    midiParser.setNoteMessageCallback(noteMessageCallback);
    midiParser.setSongPositionCallback(songPositionCallback);
    midiParser.setTransportCallback(transportCallback);
    midiParser.setBpmCallback(bpmCallback);
}

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
    ESP_ERROR_CHECK(sender.init());
    display.renderLoading();
    // wait until we start sending I2C data
    vTaskDelay(pdMS_TO_TICKS(1000));
    createMenuRenderTask();
    menuHolder.init([](const MenuState &state)
                    {
        // overwrite any pending state so we only keep the latest
        xQueueOverwrite(menuRenderQueue, &state); }, updateCallback);

    // initMidi();
}
