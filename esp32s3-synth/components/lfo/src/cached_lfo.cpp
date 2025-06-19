#include "cached_lfo.hpp"
#include "esp_log.h"
#include "lfo.hpp"
#include <cmath>
 

static const char *TAG = "CachedLFO";

CachedLFO::CachedLFO(LFO &lfoRef, uint16_t intervalTicks, uint16_t phaseOffset)
    : lfo(lfoRef),
      interval(intervalTicks),
      phase(phaseOffset % intervalTicks),
      counter(0),
      value(0.0f) {}

float CachedLFO::getValue()
{
    counter = (counter + 1) % interval;

    if (counter == phase)
    {
        value = lfo.getValue(interval);
        ESP_LOGI(TAG, "lfo gen new value %f", value);
    }
    return value;
}

void CachedLFO::setInterval(uint16_t newInterval)
{
    interval = newInterval;
    counter = 0;
    phase %= interval;
}

void CachedLFO::setPhase(uint16_t newPhase)
{
    phase = newPhase % interval;
}
