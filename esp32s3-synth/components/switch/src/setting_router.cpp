#include "setting_router.hpp"
#include "set_page.hpp"
#include "esp_log.h"
static constexpr char *TAG = "Settings Router";

using namespace settings;

SettingRouter::SettingRouter(SoundModule &soundModule) : soundModule(soundModule) {};

uint8_t SettingRouter::getMidiBpm() { return soundModule.getState().midiBpm; }

void SettingRouter::setMasterVolume(uint8_t volume)
{
    soundModule.getState().masterVolume = volume;
};

void SettingRouter::setBpmFromMidi(uint8_t bpm)
{
    this->soundModule.getState().midiBpm = bpm;
    this->soundModule.updateBpmSetting();
};

void SettingRouter::setUpdateFromUi(const FieldUpdateList &update)
{
    for (auto &u : update)
    {
        setUpdate(u);
    }
};

void SettingRouter::setTransportState(const TransportCommand &update)
{
    this->soundModule.getState().transportState = update;
};

void SettingRouter::setUpdate(const FieldUpdate &update)
{
    // Cast the raw byte to our Page enum
    Page page = static_cast<Page>(update.pageByte);
    ESP_LOGI(TAG, "Update voice %d page %s fields %d value %d", update.voiceIndex, menuPages[update.pageByte].title, update.field, update.value);

    // Dispatch to the right “setXPage” function based on which page it is:

    switch (page)
    {
    case Page::Channel:
        settings::setChannelPage(
            soundModule.getVoice(update.voiceIndex),
            update.field,
            update.value);
        break;

    case Page::Oscillator:
        settings::setOscillatorPage(
            soundModule.getVoice(update.voiceIndex),
            update.field,
            update.value);
        break;

    case Page::Filter:
        settings::setFilterPage(
            soundModule.getVoice(update.voiceIndex),
            update.field,
            update.value);
        break;

    case Page::Envelope:
        settings::setEnvelopePage(
            soundModule.getVoice(update.voiceIndex),
            update.field,
            update.value);
        break;

    case Page::Tuning:
        settings::setTuningPage(
            soundModule.getVoice(update.voiceIndex),
            update.field,
            update.value);
        break;

    case Page::PitchLFO:
        settings::setPitchLfoPage(
            soundModule.getVoice(update.voiceIndex),
            update.field,
            update.value);
        break;

    case Page::AmpLFO:
        settings::setAmpLfoPage(
            soundModule.getVoice(update.voiceIndex),
            update.field,
            update.value);
        break;

    case Page::Global:
        settings::setGlobalPage(soundModule, update.field, update.value);
        break;

    default:
        // Either log an error or ignore an out‐of‐range pageByte
        // (e.g. pageByte >= static_cast<uint8_t>(Page::_Count)).
        break;
    }
};
