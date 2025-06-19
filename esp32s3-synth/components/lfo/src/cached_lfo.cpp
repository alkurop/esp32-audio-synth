#include "cached_lfo.hpp"
#include "esp_log.h"
#include "lfo.hpp"
#include <cmath>
#include "esp_timer.h"

static const char *TAG = "CachedLFO";

CachedLFO::CachedLFO(LFO &lfoRef, uint16_t intervalTicks)
    : lfo(lfoRef),
      interval(intervalTicks),
      value(0.0f) {}

float CachedLFO::getValue()
{
    tickAccumulator++;

    if (tickAccumulator >= interval && lfo.getDepth() != 0)
    {
        tickAccumulator = 0;

        uint32_t now = esp_timer_get_time();
        uint32_t elapsed = now - lastCallTime;
        lastCallTime = now;

        lfo.advancePhaseMicroseconds(elapsed);
        value = lfo.getValue();

        // Optional debug log
        // ESP_LOGI(TAG, "LFO updated after %lu µs", elapsed);
    }

    return value;
}
