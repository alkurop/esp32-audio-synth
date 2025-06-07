#include "set_page.hpp"

using namespace settings;
using namespace protocol;

void settings::setEnvelopePage(Voice &voice, uint8_t field, int16_t value)
{
        auto fieldType = static_cast<protocol::EnvelopeField>(field);
        switch (fieldType)
        {
        case EnvelopeField::A:
                break;
        case EnvelopeField::D:
                break;
        case EnvelopeField::S:
                break;
        case EnvelopeField::R:
                /* code */
                break;
                default:break;
        }
};
