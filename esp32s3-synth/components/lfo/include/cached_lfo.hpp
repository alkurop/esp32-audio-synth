#pragma once
#include "lfo.hpp"
#include "esp_log.h"
#include <cstdint>

class CachedLFO
{
public:
    CachedLFO(LFO &lfoRef, uint16_t intervalTicks = 8, uint16_t phaseOffset = 0);
    float getValue();

private:
    LFO &lfo;
    uint16_t interval;
    uint16_t tickAccumulator = 0;
    uint32_t lastCallTime = 0;
    float value = 0.0f;
};
