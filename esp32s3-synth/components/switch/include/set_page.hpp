#pragma once
#include "sound_module.hpp"
#include <cstdint>
using namespace sound_module;

namespace settings
{
    void setChannelPage(SoundModule &sound_module, uint8_t voice_index, uint8_t field, int16_t value);
    void setOscillatorPage(SoundModule &sound_module, uint8_t voice_index, uint8_t field, int16_t value);
    void setFilterPage(SoundModule &sound_module, uint8_t voice_index, uint8_t field, int16_t value);
    void setEnvelopePage(SoundModule &sound_module, uint8_t voice_index, uint8_t field, int16_t value);
    void setTuningPage(SoundModule &sound_module, uint8_t voice_index, uint8_t field, int16_t value);
    void setFilterLfoPage(SoundModule &sound_module, uint8_t voice_index, uint8_t field, int16_t value);
    void setDetuneLfoPage(SoundModule &sound_module, uint8_t voice_index, uint8_t field, int16_t value);
}
