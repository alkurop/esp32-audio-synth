#include "sustain.hpp"
#include "esp_log.h"
static const char *TAG = "Sustain";

SustainPhase::SustainPhase(float sampleRate) : EnvelopePhase(sampleRate) {};

void SustainPhase::recalculate(uint8_t param, float /*bpm*/, float)
{
    sustainLevel = static_cast<float>(param) / static_cast<float>(envelope::MAX);
}

void SustainPhase::enter(float /*startLevel*/)
{
    ESP_LOGI(TAG, "enter level %f", sustainLevel);
    // Nothing to do — just hold sustain level
}

float SustainPhase::next()
{
    return sustainLevel;
}

float SustainPhase::currentValue() const
{
    return sustainLevel;
}

bool SustainPhase::isFinished() const
{
    return false; // Sustain never finishes until gate is released
}
