#include "set_page.hpp"

using namespace settings;
using namespace protocol;

void settings::setEnvelopePage(Voice &voice, uint8_t field, int16_t value)
{
        auto fieldType = static_cast<protocol::EnvelopeField>(field);
        auto castValue = static_cast<uint8_t>(value);
        switch (fieldType)
        {
        case EnvelopeField::A:
                voice.setAttack(castValue);
                break;
        case EnvelopeField::D:
                voice.setDecay(castValue);
                break;
        case EnvelopeField::S:
                voice.setSustain(castValue);
                break;
        case EnvelopeField::R:
                voice.setRelease(castValue);
                break;
        default:
                break;
        }
};
