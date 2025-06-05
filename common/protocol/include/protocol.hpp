#pragma once
#include <cstdint>
#include "menu_struct.hpp"

namespace protocol
{
    constexpr const uint8_t RECEIVER_ARRDESS = 0x28;
    constexpr const uint32_t i2c_frequency = 100 * 1000;
    constexpr const uint32_t I2C_CLOCK_SPEED = i2c_frequency;
    


    template <class... Ts>
    struct overloaded : Ts...
    {
        using Ts::operator()...;
    };
    template <class... Ts>
    overloaded(Ts...) -> overloaded<Ts...>;
    struct __attribute__((packed)) FieldUpdate
    {
        uint8_t voiceIndex;
        uint8_t pageByte;
        uint8_t field;
        int16_t value;
    };

    using FieldUpdateList = std::vector<FieldUpdate>;
    using UpdateCallback = std::function<void(FieldUpdateList)>;

    struct IncomingMessage
    {
        uint8_t *buffer;
        size_t length;
    };
}
