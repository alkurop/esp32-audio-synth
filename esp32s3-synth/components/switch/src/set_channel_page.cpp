#include "set_page.hpp"

using namespace protocol;
void settings::setChannelPage(SoundModule &sound_module, uint8_t voice_index, uint8_t field, int16_t value)
{
    auto &voice = sound_module.getVoices()[voice_index];
    auto fieldType = static_cast<ChannelField>(field);

    switch (fieldType)
    {
    case ChannelField::Chan:
        voice.setMidiChannel(static_cast<uint8_t>(value));
        break;
    case ChannelField::Vol:
        voice.setVolume(static_cast<uint8_t>(value));
        break;
    default:
        break;
    }
};
