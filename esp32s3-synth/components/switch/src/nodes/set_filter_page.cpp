#include "set_page.hpp"

using namespace settings;

void settings::setFilterPage(Voice &voice, uint8_t field, int16_t value)
{
    auto fieldType = static_cast<protocol::FilterField>(field);
    switch (fieldType)
    {
    case FilterField::Type:
    {
        auto castValue = static_cast<protocol::FilterType>(value);
        voice.filter.setType(castValue);
        break;
    }
    case FilterField::Cutoff:
    {
        auto castValue = static_cast<uint8_t>(value);
        voice.filter.setCutoff(castValue);
        break;
    }
    case FilterField::Resonance:
     auto castValue = static_cast<uint8_t>(value);
        voice.filter.setResonance(castValue);
        break;

    default:
        break;
    }
};
