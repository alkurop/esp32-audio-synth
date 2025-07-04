// main.cpp
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <esp_log.h>
#include "synth_config.hpp"
#include "sound_module.hpp"
#include "protocol.hpp"
#include "receiver.hpp"
#include "knob.hpp"
#include "setting_router.hpp"
#include "cached_lfo.hpp"

using namespace midi_module;
using namespace sound_module;
using namespace protocol;
using namespace ui;
static const char *TAG = "Main";

SoundModule soundModule(config);

Receiver receiver(receiverConfig);
settings::SettingRouter settingSwitch(soundModule);

Knob masterKnob(masterKnobConfig);

auto masterKnobCallback = [](uint8_t value)
{
    settingSwitch.setMasterVolume(value);
};

auto updateCallback = [](const EventList &events)
{
    for (auto &e : events)
    {
        switch (e.type)
        {
        case protocol::EventType::FieldUpdate:
            settingSwitch.setUpdateFromUi(e.fields);
            break;
        case protocol::EventType::MidiNote:
        {
            ESP_LOGD(TAG, "Midi note in: %d %d %d %d ", e.note.isNoteOn(), e.note.note, e.note.status, e.note.velocity);
            soundModule.handle_note(e.note);
        }
        break;
        }
    }
};

extern "C" void app_main()
{
    soundModule.init();
    masterKnob.init(masterKnobCallback);
    ESP_ERROR_CHECK(receiver.init(updateCallback));
}
