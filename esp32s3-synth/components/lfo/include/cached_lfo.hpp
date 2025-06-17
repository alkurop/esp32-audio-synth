#pragma once
#include "lfo.hpp"
#include <cstdint>

class CachedLFO
{
public:
     CachedLFO(LFO& lfoRef, uint16_t intervalTicks = 8, uint16_t phaseOffset = 0)
        : lfo(lfoRef),
          interval(intervalTicks),
          phase(phaseOffset % intervalTicks),
          counter(0),
          value(0.0f) {}
    float getValue()
    {
        counter = (counter + 1) % interval;

        if (counter == phase)
        {
            value = lfo.getValue(interval); // tick-based advancement
        }
        return value;
    }

    void setInterval(uint16_t newInterval)
    {
        interval = newInterval;
        counter = 0;
        phase %= interval; // make sure phase stays valid
    }

    void setPhase(uint16_t newPhase)
    {
        phase = newPhase % interval;
    }

private:
    LFO& lfo;
    uint16_t interval = 8;   // How many ticks between evaluations
    uint16_t phase = 0;      // When to evaluate within the interval
    uint16_t counter = 0;    // Current tick
    float value = 0.0f;
};
