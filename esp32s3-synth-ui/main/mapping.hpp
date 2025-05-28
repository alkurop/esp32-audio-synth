#pragma once
#include "rotary.hpp"
#include "menu_struct.hpp"

namespace ui
{

    inline void updateEncoder(
        ui::Rotary *encoder,
        const menu::EncoderRange &range)
    {
        auto &config = encoder->config;
        encoder->setRange(range.min, range.max);
        encoder->setPosition(range.value);
    }
}
