// main.cpp
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <esp_log.h>
#include "config.hpp"
#include "midi_module.hpp"
#include "sound_module.hpp"
#include "protocol.hpp"
#include "receiver.hpp"

using namespace midi_module;
using namespace sound_module;
using namespace protocol;

static const char *TAG = "Main";

SoundModule soundModule(config);
MidiParser midiParser;
MidiModule midiModule;
Receiver i2cReceiver(receiverConfig);

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

MidiNoteMessageCallback noteMessageCallback = [](const NoteMessage &note)
{
    soundModule.handle_note(note);
};

MidiSongPositionCallback songPositionCallback = [](const SongPosition &sp)
{
    ESP_LOGI(TAG, "Song Position: %d", sp.position);
};

MidiTransportCallback transportCallback = [](const TransportEvent &ev)
{
    // handle transport as needed
};

BpmCounter::BpmCallback bpmCallback = [](uint8_t bpm)
{
    // handle BPM change as needed
};

int16_t sendBpm() { return 0; };

auto updateCallback = [](const FieldUpdateList &updates)
{
    ESP_LOGI("Receiver", "Received %zu field update(s)", updates.size());
    for (const auto &u : updates)
    {
        ESP_LOGI("Receiver", "voice=%u page=%u field=%u value=%d",
                 u.voiceIndex,
                 u.pageByte,
                 u.field,
                 u.value);
    }
};

extern "C" void app_main()
{

    // // Initialize modules
    // midiModule.init(midiReadCallback);
    // soundModule.init();

    // // Set MIDI parser callbacks
    // midiParser.setControllerCallback(controllerCallback);
    // midiParser.setNoteMessageCallback(noteMessageCallback);
    // midiParser.setSongPositionCallback(songPositionCallback);
    // midiParser.setTransportCallback(transportCallback);
    // midiParser.setBpmCallback(bpmCallback);

    ESP_ERROR_CHECK(i2cReceiver.init(updateCallback, sendBpm));

    // ESP_LOGI(TAG, "Something happened");
}
