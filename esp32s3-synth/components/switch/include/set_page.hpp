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
    void setFilterCutoffLfoPage(Voice &voice, uint8_t field, int16_t value);
    void setFilterResLfoPage(Voice &voice, uint8_t field, int16_t value);
    void setPitchLfoPage(Voice &voice, uint8_t field, int16_t value);
    void setAmpLfoPage(Voice &voice, uint8_t field, int16_t value);
    void setGlobalPage(SoundModule &sound_module, uint8_t field, int16_t value);
}
