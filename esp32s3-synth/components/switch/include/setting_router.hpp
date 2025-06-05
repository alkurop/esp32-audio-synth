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
    class SettingRouter
    {
    private:
        SoundModule &soundModule;

        void setUpdate(const FieldUpdate &update);

    public:
        SettingRouter(SoundModule &soundModule);
        void setMasterVolume(uint8_t volume);
        void setBpmFromMidi(uint8_t bpm);
        void setUpdateFromUi(const FieldUpdateList &update);
        uint8_t getMidiBpm();
        void setTransportState(const TransportCommand &setTransportState);
    };

}
