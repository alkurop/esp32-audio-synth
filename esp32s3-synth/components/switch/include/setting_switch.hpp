#pragma once
#include "sound_module.hpp"
#include "protocol.hpp"
#include "menu_struct.hpp"
#include "midi_parser.hpp"
#include <cstdint>

using namespace protocol;
using namespace sound_module;
using namespace midi_module;

namespace settings

{
    class SettingSwitch
    {
    private:
        SoundModule &soundModule;
        uint8_t midiBpm;
        uint16_t settingBpm;
        bool isSync;
        TransportCommand transportState;

        void setUpdate(const FieldUpdate &update);
        void setGlobalPage(uint8_t field, int16_t value);

    public:
        SettingSwitch(SoundModule &soundModule);
        void setMasterVolume(uint8_t volume);
        void setBpmFromMidi(uint8_t bpm);
        void setUpdateFromUi(const FieldUpdateList &update);
        uint8_t getMidiBpm() { return midiBpm; };

        void setTransportState(const TransportCommand &setTransportState);
    };

}
