#include "decay.hpp"
#include <cmath>
#include <algorithm>
#include <esp_log.h>
#define TAG "Decay"

DecayPhase::DecayPhase(float sampleRate) : EnvelopePhase(sampleRate)
{
}

void DecayPhase::recalculate(uint8_t param, float bpm, float sustainLevel)
{
    targetLevel = sustainLevel;
    float beats = calcBeats(param, bpm);
    totalSamples = beatsToSamples(beats, bpm);
    reciprocal = (totalSamples > 0.0f ? 1.0f / totalSamples : 0.0f);

    if (!finished && totalSamples > 0.0f && startLevel > targetLevel)
    {
        float distance = startLevel - targetLevel;
        float progress = (startLevel - currentLevel) / distance;
        cursor = std::clamp(progress * totalSamples, 0.0f, totalSamples);
    }
}

void DecayPhase::enter(float level)
{
    // ESP_LOGI(TAG, "enter level %f", level);

    startLevel = level;
    cursor = 0.0f;
    currentLevel = level;

    // If the start is already below or equal to target, skip decay
    finished = (startLevel <= targetLevel);
}

float DecayPhase::next()
{
    if (finished)
        return targetLevel;

    float t = cursor * reciprocal;
    currentLevel = startLevel + (targetLevel - startLevel) * t;  

    ++cursor;
    if (cursor >= totalSamples || currentLevel <= targetLevel)
    {
        currentLevel = targetLevel;
        finished = true;
    }

    return currentLevel;
}
float DecayPhase::currentValue() const
{
    return currentLevel;
}

bool DecayPhase::isFinished() const
{
    return finished;
}
