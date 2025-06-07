#include "set_page.hpp"

using namespace settings;

void settings::setAmpLfoPage(Voice &voice, uint8_t field, int16_t value)
{
    auto fieldType = static_cast<protocol::LFOField>(field);
};
