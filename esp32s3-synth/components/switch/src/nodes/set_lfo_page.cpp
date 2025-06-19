#include "set_page.hpp"

using namespace settings;
using namespace protocol;

void setLfo(LFO &lfo, uint8_t field, int16_t value)
{
    auto fieldType = static_cast<protocol::LFOField>(field);
    switch (fieldType)
    {
    case LFOField::Form:
    {
        auto castValue = static_cast<protocol::LfoWaveform>(value);
        lfo.setWaveform(castValue);
        break;
    }
    case LFOField::Subdiv:
    {
        auto castValue = static_cast<protocol::LfoSubdivision>(value);
        lfo.setSubdivision(castValue);
        break;
    }
    case LFOField::Depth:
    {
        auto castValue = static_cast<uint8_t>(value);
        lfo.setDepth(castValue);
        break;
    }

    default:
        break;
    }
}

void settings::setAmpLfoPage(Voice &voice, uint8_t field, int16_t value)
{
    // setLfo(voice.ampLfo, field, value);
};

void settings::setPitchLfoPage(Voice &voice, uint8_t field, int16_t value)
{
    setLfo(voice.pitchLfo, field, value);
};

void settings::setFilterCutoffLfoPage(Voice &voice, uint8_t field, int16_t value)
{
    // auto lfo = voice.filter.cutoffLfo;
    // setLfo(lfo, field, value);
};

void settings::setFilterResLfoPage(Voice &voice, uint8_t field, int16_t value)
{
    // auto lfo = voice.filter.resonanceLfo;
    // setLfo(lfo, field, value);
};
