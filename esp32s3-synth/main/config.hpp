#pragma once
#include "midi_module.hpp"
#include "sound_module.hpp"
#include "receiver.hpp"
#include "protocol.hpp"
#include "master_knob.hpp"

using namespace midi_module;
using namespace sound_module;
using namespace protocol;
using namespace ui;

// ESP32-S3 Pin Mapping for I2S
#define I2S_BCK_IO GPIO_NUM_17
#define I2S_WS_IO GPIO_NUM_18
#define I2S_DO_IO GPIO_NUM_21

// ESP32-S3 Pin Mapping for I2C
#define SDA_PIN   GPIO_NUM_13  // red
#define SCL_PIN  GPIO_NUM_14 // yellow

#define MASTER_KNOB_PIN GPIO_NUM_4

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

MasterKnobConfig masterKnobConfig = {
    .pin  = MASTER_KNOB_PIN,
    .adc_channel = ADC_CHANNEL_3,
    .adc_unit= ADC_UNIT_1,
};
