#pragma once
#include "sound_module.hpp"
#include "protocol.hpp"
#include "menu_struct.hpp"
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
        void setBpmFromMidi(uint16_t bpm);
        void setUpdateFromUi(const FieldUpdateList &update);
        void setTransportState(const TransportCommand &setTransportState);
    };

}
