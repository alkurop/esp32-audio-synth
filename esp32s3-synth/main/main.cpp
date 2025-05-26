// main.cpp
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <esp_log.h>
#include "midi_module.hpp"
#include "sound_module.hpp"

using namespace midi_module;
using namespace sound_module;

static const char *TAG = "Main";

// ESP32-S3 Pin Mapping for I2S
#define I2S_BCK_IO GPIO_NUM_17
#define I2S_WS_IO GPIO_NUM_18
#define I2S_DO_IO GPIO_NUM_21

// Configure sound engine
SoundConfig config{
    .sampleRate = 96000,
    .tableSize = 1024,
    .amplitude = 16000,
    .bufferSize = 128,
    .numVoices = 8,
    .i2s = {
        .bclk_io = I2S_BCK_IO,
        .lrclk_io = I2S_WS_IO,
        .data_io = I2S_DO_IO},
};

SoundModule soundModule(config);
MidiParser midiParser;
MidiModule midiModule;

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

extern "C" void app_main()
{

    // Initialize modules
    midiModule.init(midiReadCallback);
    soundModule.init();

    // Set MIDI parser callbacks
    midiParser.setControllerCallback(controllerCallback);
    midiParser.setNoteMessageCallback(noteMessageCallback);
    midiParser.setSongPositionCallback(songPositionCallback);
    midiParser.setTransportCallback(transportCallback);
    midiParser.setBpmCallback(bpmCallback);
}
