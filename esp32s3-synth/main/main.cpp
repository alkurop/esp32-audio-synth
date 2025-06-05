// main.cpp
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <esp_log.h>
#include "config.hpp"
#include "midi_module.hpp"
#include "sound_module.hpp"
#include "protocol.hpp"
#include "receiver.hpp"
#include "knob.hpp"
#include "setting_router.hpp"

using namespace midi_module;
using namespace sound_module;
using namespace protocol;
using namespace ui;
static const char *TAG = "Main";

SoundModule soundModule(config);
MidiParser midiParser;
MidiModule midiModule;
Receiver i2cReceiver(receiverConfig);
settings::SettingRouter settingSwitch(soundModule);

Knob masterKnob(masterKnobConfig);

auto masterKnobCallback = [](uint8_t value)
{ settingSwitch.setMasterVolume(value); };

// MIDI packet callback
MidiReadCallback midiReadCallback = [](const uint8_t packet[4])
{ midiParser.feed(packet); };

// MIDI event callbacks
MidiControllerCallback controllerCallback = [](const ControllerChange &cc)
{
    // handle CC as needed
};

MidiNoteMessageCallback noteMessageCallback = [](const NoteMessage &note)
{ soundModule.handle_note(note); };

MidiSongPositionCallback songPositionCallback = [](const SongPosition &sp)
{ ESP_LOGI(TAG, "Song Position: %d", sp.position); };

MidiTransportCallback transportCallback = [](const TransportEvent &ev)
{ settingSwitch.setTransportState(ev.command); };

BpmCounter::BpmCallback bpmCallback = [](uint8_t bpm)
{
    return settingSwitch.getMidiBpm();
};

uint16_t sendBpm() { return settingSwitch.getMidiBpm(); };

auto updateCallback = [](const FieldUpdateList &updates)
{ settingSwitch.setUpdateFromUi(updates); };

extern "C" void app_main()
{

    // // // Initialize modules
    // midiModule.init(midiReadCallback);
    // soundModule.init();

    // // Set MIDI parser callbacks
    // midiParser.setControllerCallback(controllerCallback);
    // midiParser.setNoteMessageCallback(noteMessageCallback);
    // midiParser.setSongPositionCallback(songPositionCallback);
    // midiParser.setTransportCallback(transportCallback);
    // midiParser.setBpmCallback(bpmCallback);

    masterKnob.init(masterKnobCallback);
    ESP_ERROR_CHECK(i2cReceiver.init(updateCallback, sendBpm));
}
