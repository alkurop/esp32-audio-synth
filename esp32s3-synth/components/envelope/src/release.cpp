#include "release.hpp"
#include <cmath>
#include <esp_log.h>
#include <algorithm>
static const char *TAG = "Release";

ReleasePhase::ReleasePhase(float sampleRate) : EnvelopePhase(sampleRate)
{
}

void ReleasePhase::recalculate(uint8_t param, float bpm, float /*unused*/)
{
    float beats = calcBeats(param, bpm);
    totalSamples = beatsToSamples(beats, bpm);
    reciprocal = (totalSamples > 0.0f ? 1.0f / totalSamples : 0.0f);

    if (!finished && totalSamples > 0.0f && startLevel > 0.0f)
    {
        float progress = (startLevel - currentLevel) / startLevel;
        cursor = std::clamp(progress * totalSamples, 0.0f, totalSamples);
    }
}

void ReleasePhase::enter(float level)
{
    ESP_LOGI(TAG, "enter level %f", level);
    startLevel = level;
    currentLevel = level;
    cursor = 0.0f;
    finished = (level <= 0.0f || totalSamples <= 0.0f);
}

float ReleasePhase::next()
{
    if (finished)
        return 0.0f;

    float t = cursor * reciprocal;
    // currentLevel = startLevel * (1.0f - t);  linear, next line is exponential
    currentLevel = startLevel * std::pow(0.001f, t);

    cursor += 1.0f;

    if (cursor >= totalSamples || currentLevel <= 0.0f)
    {
        currentLevel = 0.0f;
        finished = true;
    }

    return currentLevel;
}

float ReleasePhase::currentValue() const
{
    return currentLevel;
}

bool ReleasePhase::isFinished() const
{
    return finished;
}
