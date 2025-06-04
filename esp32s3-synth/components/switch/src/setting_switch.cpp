#include "setting_switch.hpp"
#include "set_page.hpp"

using namespace settings;

SettingSwitch::SettingSwitch(SoundModule &soundModule) : soundModule(soundModule) {};

void SettingSwitch::setMasterVolume(uint8_t volume) {};

void SettingSwitch::setBpmFromMidi(uint8_t bpm)
{
    this->midiBpm = bpm;
};

void SettingSwitch::setUpdateFromUi(const FieldUpdateList &update)
{
    for (auto &u : update)
    {
        setUpdate(u);
    }
};

void SettingSwitch::setTransportState(const TransportCommand &update)
{
    this->transportState = update;
};

void SettingSwitch::setUpdate(const FieldUpdate &update)
{
    // Cast the raw byte to our Page enum
    Page page = static_cast<Page>(update.pageByte);

    // Dispatch to the right “setXPage” function based on which page it is:
    switch (page)
    {
    case Page::Channel:
        settings::setChannelPage(
            soundModule,
            update.voiceIndex,
            update.field,
            update.value);
        break;

    case Page::Oscillator:
        settings::setOscillatorPage(
            soundModule,
            update.voiceIndex,
            update.field,
            update.value);
        break;

    case Page::Filter:
        settings::setFilterPage(
            soundModule,
            update.voiceIndex,
            update.field,
            update.value);
        break;

    case Page::Envelope:
        settings::setEnvelopePage(
            soundModule,
            update.voiceIndex,
            update.field,
            update.value);
        break;

    case Page::Tuning:
        settings::setTuningPage(
            soundModule,
            update.voiceIndex,
            update.field,
            update.value);
        break;

    case Page::FilterLFO:
        settings::setFilterLfoPage(
            soundModule,
            update.voiceIndex,
            update.field,
            update.value);
        break;

    case Page::DetuneLFO:
        settings::setDetuneLfoPage(
            soundModule,
            update.voiceIndex,
            update.field,
            update.value);
        break;

    case Page::Global:
        setGlobalPage(update.field, update.value);
        break;

    default:
        // Either log an error or ignore an out‐of‐range pageByte
        // (e.g. pageByte >= static_cast<uint8_t>(Page::_Count)).
        break;
    }
};

void SettingSwitch::setGlobalPage(uint8_t field, int16_t value) {};
