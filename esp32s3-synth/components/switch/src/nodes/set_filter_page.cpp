#include "set_page.hpp"

using namespace settings;

void settings::setFilterPage(Voice &voice, uint8_t field, int16_t value)
{
    auto fieldType = static_cast<protocol::FilterField>(field);
};
