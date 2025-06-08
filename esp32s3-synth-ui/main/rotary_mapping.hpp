#pragma once
#include "rotary.hpp"
#include "menu_struct.hpp"
using namespace protocol;
namespace ui
{

    inline void updateEncoder(
        ui::Rotary *encoder,
        const EncoderRange &range)
    {
        encoder->setRange(range.min, range.max);
        encoder->setPosition(range.value);
        encoder->setIncrement(range.increment);
    }
}
