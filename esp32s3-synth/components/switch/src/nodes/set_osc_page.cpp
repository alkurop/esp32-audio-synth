#include "set_page.hpp"
#include <algorithm>

using namespace settings;

void settings::setOscillatorPage(Voice &voice, uint8_t field, int16_t value)
{
    auto fieldType = static_cast<protocol::OscillatorField>(field);
    for (auto &sound : voice.getSounds())
    {
        switch (fieldType)
        {
        case OscillatorField::Shape:
        {
            auto idx = std::clamp<int16_t>(
                value,
                0,
                static_cast<int16_t>(protocol::OscillatorShape::_Count) - 1);
            // Cast the clamped index to your shape enum
            sound.set_shape(static_cast<protocol::OscillatorShape>(idx));
            break;
        }
        break;

        case OscillatorField::PWM:
        {
            auto idx = std::clamp<int16_t>(
                value,
                0,
                OSCILLATOR_PWM_MAX);
            sound.set_pwm(static_cast<uint8_t>(idx));
            break;
        }
        case OscillatorField::Sync:
        {
            sound.set_sync(static_cast<bool>(value));
            break;
        }
        default:
            break;
        }
    }
};
