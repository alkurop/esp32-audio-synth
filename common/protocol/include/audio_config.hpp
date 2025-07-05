#pragma once

namespace protocol
{
    constexpr const float VELOCITY_GLOBAL_SCALER = 1;
    constexpr const int LOOKUP_TABLE_SIZE = 1024;
    constexpr const int NUM_VOICES = 3;
    constexpr const int NUM_SOUNDS = 5;
    constexpr const int SAMPLE_RATE = 48000;
    constexpr const int AMPLITUDE = 24000;
    constexpr const int BUFFER_SIZE = 512;
    constexpr const int BPM_DEFAULT = 120;

    constexpr const int RECEIVER_ARRDESS = 0x28;
    constexpr const int I2C_CLOCK_SPEED = 1000 * 1000;

}
