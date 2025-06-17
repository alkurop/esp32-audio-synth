#include "set_page.hpp"

using namespace settings;

void settings::setTuningPage(Voice &voice, uint8_t field, int16_t value)
{
    auto fieldType = static_cast<protocol::TuningField>(field);
    switch (fieldType)
    {
    case TuningField::Octave:
        break;
    case TuningField::Semitone:
        break;
    case TuningField::FineTune:
        break;
    default:
        break;
    }
    voice.updatePitchOffset();
};
