#include "attack.hpp"
#include "esp_log.h"
#include <cmath>
static const char *TAG = "Attack";

AttackPhase::AttackPhase(float sampleRate) : EnvelopePhase(sampleRate)
{
}

void AttackPhase::recalculate(uint8_t param, float bpm, float)
{
    float beats = calcBeats(param, bpm);
    totalSamples = beatsToSamples(beats, bpm);
    reciprocal = (totalSamples > 0.0f) ? (1.0f / totalSamples) : 0.0f;

    if (!finished && totalSamples > 0.0f)
    {
        cursor = currentLevel * totalSamples;    // safer numerically
        cursor = std::min(cursor, totalSamples); // clamp
    }
}

void AttackPhase::enter(float /*startLevel*/)
{
    ESP_LOGI(TAG."enter");
    cursor = 0.0f;
    currentLevel = 0.0f;
}

float AttackPhase::next()
{
    if (finished)
        return 1.0f;

    //     currentLevel = cursor * reciprocal; this is linear. next is exponential

    float t = cursor * reciprocal;
    constexpr float kAttackStartRatio = 0.001f;
    currentLevel = 1.0f - std::pow(kAttackStartRatio, t);

    cursor += 1.0f;
    if (cursor >= totalSamples || currentLevel >= 1.0f)
    {
        currentLevel = 1.0f;
        finished = true;
    }

    return currentLevel;
}

float AttackPhase::currentValue() const
{
    return currentLevel;
}

bool AttackPhase::isFinished() const
{
    return cursor >= totalSamples;
}
