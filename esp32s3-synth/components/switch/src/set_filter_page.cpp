#include "set_page.hpp"

using namespace settings;

void settings::setFilterPage(SoundModule &sound_module, uint8_t voice_index, uint8_t field, int16_t value)
{
    auto fieldType = static_cast<protocol::FilterField>(field);
};
