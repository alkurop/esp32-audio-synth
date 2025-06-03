#include "setting_switch.hpp"
#include "set_page_channel.hpp"
#include "set_page_detune_lfo.hpp"
#include "set_page_env.hpp"
#include "set_page_filter_lfo.hpp"
#include "set_page_filter.hpp"
#include "set_page_osc.hpp"
#include "set_page_tuning.hpp"

using namespace settings;

SettingSwitch::SettingSwitch(SoundModule *soundModule) : soundModule(soundModule) {};

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
    auto page = static_cast<Page>(update.pageByte);
};
