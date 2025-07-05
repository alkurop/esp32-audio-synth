#include "rotary.hpp"
#include <algorithm>
#define COUNT_MIN -100
#define COUNT_MAX 100
#define STEPS_PER_INDENT 4 ///< Gray-code edges per detent (EC11-style = 4)
using namespace ui;
#include "esp_intr_alloc.h"

// GPIO ISR handle - static so C linkage
static void IRAM_ATTR gpio_isr(void *arg)
{
    auto self = static_cast<Rotary *>(arg);
    BaseType_t hpw = pdFALSE;
    int8_t delta = 1;
    xQueueSendFromISR(self->eventQueue, &delta, &hpw);
}

Rotary::Rotary(const RotaryConfig &cfg)
    : config(cfg)
{
}

void Rotary::init(const RotaryCallback &cb)
{
    callback = cb;
    // Create queue for deltas
    eventQueue = xQueueCreate(10, sizeof(int8_t));
    configASSERT(eventQueue);

    initPCNT();
    initGPIOInterrupt();
    startTask();
}

void Rotary::initPCNT()
{
    // Initialize PCNT to keep raw count (optional, can be omitted if not used)
    pcnt_unit_config_t ucfg = {};
    ucfg.low_limit = INT16_MIN;
    ucfg.high_limit = INT16_MAX;
    ucfg.intr_priority = 1;
    ESP_ERROR_CHECK(pcnt_new_unit(&ucfg, &unit));

    pcnt_chan_config_t chan_cfg = {};
    chan_cfg.edge_gpio_num = config.pin_clk;
    chan_cfg.level_gpio_num = config.pin_dt;
    pcnt_channel_handle_t channel_clk = nullptr;
    pcnt_channel_handle_t channel_dt = nullptr;

    ESP_ERROR_CHECK(pcnt_new_channel(unit, &chan_cfg, &channel_clk));
    chan_cfg.edge_gpio_num = config.pin_dt;
    chan_cfg.level_gpio_num = config.pin_clk;
    ESP_ERROR_CHECK(pcnt_new_channel(unit, &chan_cfg, &channel_dt));

    ESP_ERROR_CHECK(pcnt_channel_set_edge_action(channel_clk,
                                                 PCNT_CHANNEL_EDGE_ACTION_DECREASE,
                                                 PCNT_CHANNEL_EDGE_ACTION_INCREASE));
    ESP_ERROR_CHECK(pcnt_channel_set_level_action(channel_clk,
                                                  PCNT_CHANNEL_LEVEL_ACTION_KEEP,
                                                  PCNT_CHANNEL_LEVEL_ACTION_INVERSE));
    ESP_ERROR_CHECK(pcnt_channel_set_edge_action(channel_dt,
                                                 PCNT_CHANNEL_EDGE_ACTION_INCREASE,
                                                 PCNT_CHANNEL_EDGE_ACTION_DECREASE));
    ESP_ERROR_CHECK(pcnt_channel_set_level_action(channel_dt,
                                                  PCNT_CHANNEL_LEVEL_ACTION_KEEP,
                                                  PCNT_CHANNEL_LEVEL_ACTION_INVERSE));

    pcnt_glitch_filter_config_t filter = {.max_glitch_ns = config.glitchFilterNs};
    ESP_ERROR_CHECK(pcnt_unit_set_glitch_filter(unit, &filter));

    ESP_ERROR_CHECK(pcnt_unit_enable(unit));
    ESP_ERROR_CHECK(pcnt_unit_clear_count(unit));
    ESP_ERROR_CHECK(pcnt_unit_start(unit));
}

void Rotary::initGPIOInterrupt()
{
    // Configure CLK and DT pins as inputs with pull-up
    gpio_config_t io_conf = {};
    io_conf.pin_bit_mask = (1ULL << config.pin_clk) | (1ULL << config.pin_dt);
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.intr_type = GPIO_INTR_ANYEDGE; // trigger on both rising and falling edges
    ESP_ERROR_CHECK(gpio_config(&io_conf));

    // Install GPIO ISR service
    gpio_install_isr_service(ESP_INTR_FLAG_LOWMED);
    // Attach ISR to CLK pin
    ESP_ERROR_CHECK(gpio_isr_handler_add(
        config.pin_clk,
        gpio_isr,
        this));
}
void Rotary::startTask()
{
    xTaskCreatePinnedToCore([](void *arg)
                            { static_cast<Rotary *>(arg)->processEvents(); }, "rotary_task", 4096, this, 5, nullptr, 0);
}

void Rotary::processEvents()
{
    int event_count = 0, rawCount = 0, prevCount = 0;

    while (true)
    {
        // Block until an event comes in
        if (xQueueReceive(eventQueue, &event_count, portMAX_DELAY) == pdTRUE)
        {
            // Process the first event (we already have it)
            do
            {
                // Read the raw PCNT count and compute detents
                pcnt_unit_get_count(unit, &rawCount);
                int detents = rawCount / STEPS_PER_INDENT;

                if (detents != prevCount)
                {
                    int delta = detents - prevCount;
                    prevCount = detents;

                    int next = static_cast<int>(currentPosition) + (delta > 0 ? config.increment : -static_cast<int>(config.increment));

                    if (config.wrapAround)
                    {
                        if (next > static_cast<int>(config.maxValue))
                            currentPosition = config.minValue;
                        else if (next < static_cast<int>(config.minValue))
                            currentPosition = config.maxValue;
                        else
                            currentPosition = static_cast<int16_t>(next);
                    }
                    else
                    {
                        next = std::clamp(next,
                                          static_cast<int>(config.minValue),
                                          static_cast<int>(config.maxValue));
                        currentPosition = static_cast<int16_t>(next);
                    }

                    if (callback)
                        callback(config.id, currentPosition);
                }

                // Try to pull the next event (non-blocking)
            } while (xQueueReceive(eventQueue, &event_count, 0) == pdTRUE);
        }
    }
}
