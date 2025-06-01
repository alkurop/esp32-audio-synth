#pragma once

#include "rotary.hpp"
#include "button.hpp"
#include "ssd1306.hpp"
#include "menu.hpp"
#include "encoder_range.hpp"
#include "mapping.hpp"
#include "config.hpp"
#include "sender.hpp"
#include "protocol.hpp"

#define B0 GPIO_NUM_9
#define B1 GPIO_NUM_10
#define B2 GPIO_NUM_11
#define B3 GPIO_NUM_12
#define VOICE_COUNT 8
#define ENCODER_COUNT 4
#define RENDER_TASK_STACK 8 * 1024 // 8 KB

using namespace ui;
using namespace menu;
using namespace i2c;
using namespace protocol;

static const char *TAG = "Main";

SenderConfig senderConfig = {
    .sda_pin = GPIO_NUM_1,
    .scl_pin = GPIO_NUM_2,
    .i2c_port = I2C_NUM_1,
    .receiver_address = RECEIVER_ARRDESS};

SSD1306Config displayConfig = {
    .sda_pin = GPIO_NUM_14,
    .scl_pin = GPIO_NUM_13,
    .width = 128,
    .height = 64,
    .i2c_port = I2C_NUM_0,
    .i2c_addr = 0x3C};

RotaryConfig cfg0 = {
    .id = 0,
    .pin_clk = GPIO_NUM_41,
    .pin_dt = GPIO_NUM_40,
    .increment = 1,
};

RotaryConfig cfg1 = {
    .id = 1,
    .pin_clk = GPIO_NUM_39,
    .pin_dt = GPIO_NUM_38,
    .increment = 1,
};

RotaryConfig cfg2 = {
    .id = 2,
    .pin_clk = GPIO_NUM_45,
    .pin_dt = GPIO_NUM_48,
    .increment = 1,
};

RotaryConfig cfg3 = {
    .id = 3,
    .pin_clk = GPIO_NUM_47,
    .pin_dt = GPIO_NUM_21,
    .increment = 1,
};
