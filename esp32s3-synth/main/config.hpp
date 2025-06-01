#pragma once
#include "midi_module.hpp"
#include "sound_module.hpp"
#include "receiver.hpp"
#include "protocol.hpp"

using namespace midi_module;
using namespace sound_module;
using namespace protocol;

// ESP32-S3 Pin Mapping for I2S
#define I2S_BCK_IO GPIO_NUM_17
#define I2S_WS_IO GPIO_NUM_18
#define I2S_DO_IO GPIO_NUM_21

// ESP32-S3 Pin Mapping for I2C
#define SDA_PIN   GPIO_NUM_14
#define SCL_PIN  GPIO_NUM_13

// Configure sound engine
SoundConfig config{
    .sampleRate = 96000,
    .tableSize = 1024,
    .amplitude = 16000,
    .bufferSize = 128,
    .numVoices = 8,
    .i2s = {
        .bclk_io = I2S_BCK_IO,
        .lrclk_io = I2S_WS_IO,
        .data_io = I2S_DO_IO},
};

ReceiverConfig receiverConfig = {
    .sda_pin = SDA_PIN,
    .scl_pin = SCL_PIN,
    .i2c_port = I2C_NUM_0,
    .receiver_address = RECEIVER_ARRDESS,
};
