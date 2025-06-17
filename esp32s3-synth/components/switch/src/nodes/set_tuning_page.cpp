#include "set_page.hpp"

using namespace settings;

void settings::setTuningPage(Voice &voice, uint8_t field, int16_t value)
{
    auto fieldType = static_cast<protocol::TuningField>(field);
    switch (fieldType)
    {
    case TuningField::Octave:
        voice.pitchSettings.transpose_octave = value;
        break;
    case TuningField::Semitone:
        voice.pitchSettings.transpose_semitones = value;
        break;
    case TuningField::FineTune:
        voice.pitchSettings.fine_tuning = value;
        break;

    default:
        break;
    }
    voice.updatePitchOffset();
};
