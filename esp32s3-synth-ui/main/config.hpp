#pragma once

#include "rotary.hpp"
#include "button.hpp"
#include "display.hpp"
#include "menu.hpp"
#include "encoder_range.hpp"
#include "config.hpp"
#include "sender.hpp"
#include "protocol.hpp"

#define B0 GPIO_NUM_9
#define B1 GPIO_NUM_10
#define B2 GPIO_NUM_11
#define B3 GPIO_NUM_12

// ESP32-S3 Pin Mapping for protocol I2C
#define PROTOCOL_SDA_PIN GPIO_NUM_1
#define PROTOCOL_SCL_PIN GPIO_NUM_2
#define PROTOCOL_I2C_PORT I2C_NUM_1

// ESP32-S3 Pin Mapping for display I2C
#define DISPLAY_SDA_PIN GPIO_NUM_14
#define DISPLAY_SCL_PIN GPIO_NUM_13
#define DISPLAY_I2C_PORT I2C_NUM_0
#define DISPLAY_ADDRESS 0x3C

// Rotary encoder 0
#define ROTARY0_CLK GPIO_NUM_41
#define ROTARY0_DT  GPIO_NUM_40

// Rotary encoder 1
#define ROTARY1_CLK GPIO_NUM_39
#define ROTARY1_DT  GPIO_NUM_38

// Rotary encoder 2
#define ROTARY2_CLK GPIO_NUM_45
#define ROTARY2_DT  GPIO_NUM_48

// Rotary encoder 3
#define ROTARY3_CLK GPIO_NUM_47
#define ROTARY3_DT  GPIO_NUM_21

#define VOICE_COUNT 8
#define ENCODER_COUNT 4
#define RENDER_TASK_STACK 8 * 1024 // 8 KB

using namespace ui;
using namespace menu;
using namespace protocol;

static const char *TAG = "Main";

SenderConfig senderConfig = {
    .sda_pin = PROTOCOL_SDA_PIN,
    .scl_pin = PROTOCOL_SCL_PIN,
    .i2c_port = PROTOCOL_I2C_PORT,
    .receiver_address = RECEIVER_ARRDESS,
    .clock_speed= I2C_CLOCK_SPEED
    };

SSD1306Config displayConfig = {
    .sda_pin = DISPLAY_SDA_PIN,
    .scl_pin = DISPLAY_SCL_PIN,
    .width = 128,
    .height = 64,
    .i2c_port = DISPLAY_I2C_PORT,
    .i2c_addr = DISPLAY_ADDRESS};

RotaryConfig cfg0 = {
    .id = 0,
    .pin_clk = ROTARY0_CLK,
    .pin_dt = ROTARY0_DT,
    .increment = 1,
};

RotaryConfig cfg1 = {
    .id = 1,
    .pin_clk = ROTARY1_CLK,
    .pin_dt = ROTARY1_DT,
    .increment = 1,
};

RotaryConfig cfg2 = {
    .id = 2,
    .pin_clk = ROTARY2_CLK,
    .pin_dt = ROTARY2_DT,
    .increment = 1,
};

RotaryConfig cfg3 = {
    .id = 3,
    .pin_clk = ROTARY3_CLK,
    .pin_dt = ROTARY3_DT,
    .increment = 1,
};
