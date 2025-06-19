#pragma once
#include "lfo.hpp"
#include "esp_log.h"
#include <cstdint>

class CachedLFO
{
public:
    CachedLFO(LFO &lfoRef, uint16_t intervalTicks = 8, uint16_t phaseOffset = 0);
    float getValue();
    void setInterval(uint16_t newInterval);
    void setPhase(uint16_t newPhase);

private:
    LFO &lfo;
    uint16_t interval;
    uint16_t phase;
    uint16_t counter;
    float value;
};
