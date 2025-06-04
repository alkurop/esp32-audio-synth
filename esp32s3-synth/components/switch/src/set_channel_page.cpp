#include "set_page.hpp"

void settings::setChannelPage(SoundModule &sound_module, uint8_t voice_index, uint8_t field, int16_t value)
{
    auto fieldType = static_cast<protocol::ChannelField>(field);
};
