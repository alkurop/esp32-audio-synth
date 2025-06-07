#pragma once
#include "sound_module.hpp"
#include "protocol.hpp"
#include <cstdint>
using namespace sound_module;

namespace settings
{
    void setChannelPage(Voice &voice, uint8_t field, int16_t value);
    void setOscillatorPage(Voice &voice, uint8_t field, int16_t value);
    void setFilterPage(Voice &voice, uint8_t field, int16_t value);
    void setEnvelopePage(Voice &voice, uint8_t field, int16_t value);
    void setTuningPage(Voice &voice, uint8_t field, int16_t value);
    void setFilterLfoPage(Voice &voice, uint8_t field, int16_t value);
    void setDetuneLfoPage(Voice &voice, uint8_t field, int16_t value);
    void setGlobalPage(SoundModule &sound_module, uint8_t field, int16_t value);
}
