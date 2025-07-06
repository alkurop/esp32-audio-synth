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
#include "create_send_events.hpp"
#include "render.hpp"

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

RenderTaskCtx renderTaskContext = {.display = &display, .encoders = encoders, .encoderCount = ENCODER_COUNT};

Button buttonDown;
Button buttonUp;
Button buttonRight;
Button buttonLeft;
Sender sender(senderConfig);

Menu menuHolder(protocol::NUM_VOICES);

auto midiReadCallback = [](const uint8_t packet[4])
{ midiParser.feed(packet); };

auto controllerCallback = [](const ControllerChange &cc)
{ ESP_LOGI(TAG, "Controller Change: %d %d", cc.controller, cc.value); };

auto noteMessageCallback = [](const MidiNoteEvent &note)
{
    auto events = createNoteEventList(note);
    sender.send(events);
};

auto songPositionCallback = [](const SongPosition &sp)
{ ESP_LOGI(TAG, "Song Position: %d", sp.position); };

auto transportCallback = [](const TransportEvent &ev)
{ ESP_LOGI(TAG, "transport Position: %d", static_cast<int>(ev.command)); };

auto bpmCallback = [](uint16_t bpm)
{
    auto events = createBpmEventList(bpm);
    sender.send(events);
};

auto rotaryCallback = [](uint8_t id, int16_t newValue)
{ menuHolder.rotateKnob(id, newValue); };

auto left = [](uint8_t number, bool pressed)
{ if (pressed) menuHolder.exitPage(); };

auto right = [](uint8_t number, bool pressed)
{ if (pressed) menuHolder.enterMenuPage(); };

auto up = [](uint8_t number, bool pressed)
{ if (pressed) menuHolder.voiceUp(); };

auto down = [](uint8_t number, bool pressed)
{ if (pressed) menuHolder.voiceDown(); };

auto updateCallback = [](const FieldUpdateList &updates)
{
    auto events = createFileUpdateEventList(updates);
    sender.send(events);
};

auto displayCallback = [](const MenuState &state)
{ xQueueOverwrite(menuRenderQueue, &state); };

void initMidi()
{
    midiModule.init(midiReadCallback);
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

    ESP_ERROR_CHECK(buttonDown.init(B0, 0, down));
    ESP_ERROR_CHECK(buttonUp.init(B1, 1, up));
    ESP_ERROR_CHECK(buttonRight.init(B2, 2, right));
    ESP_ERROR_CHECK(buttonLeft.init(B3, 3, left));

    buttonDown.install();
    buttonUp.install();
    buttonRight.install();
    buttonLeft.install();

    ESP_ERROR_CHECK(display.init());
    ESP_ERROR_CHECK(sender.init());

    display.renderLoading();
    createMenuRenderTask(&renderTaskContext);

    menuHolder.init(displayCallback, updateCallback);
    initMidi();
}
