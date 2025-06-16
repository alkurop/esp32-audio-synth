#include "bpm_counter.hpp"

namespace midi_module
{

BpmCounter::BpmCounter() = default;

void BpmCounter::setCallback(BpmCallback callback)
{
    bpmCallback = callback;
}

void BpmCounter::stop()
{
    isActive = false;
    lastBeatTimestamp = 0;
    clockCount = 0;
    // leave currentBpm and history intact
}

void BpmCounter::start()
{
    isActive = true;
    lastBeatTimestamp = 0;
    clockCount = 0;
    // resume using existing history and currentBpm
}

void BpmCounter::onClockTick(uint64_t timestamp_us)
{
    if (!bpmCallback || !isActive)
        return;

    clockCount++;
    if (clockCount < clocksPerBeat)
        return;

    clockCount = 0;
    if (lastBeatTimestamp == 0) {
        lastBeatTimestamp = timestamp_us;
        return;
    }

    uint64_t delta_us = timestamp_us - lastBeatTimestamp;
    lastBeatTimestamp = timestamp_us;
    if (delta_us == 0)
        return;

    float instBpm = 60000000.0f / static_cast<float>(delta_us);

    // moving average
    bpmSum -= bpmHistory[bpmIndex];
    bpmHistory[bpmIndex] = instBpm;
    bpmSum += instBpm;
    bpmIndex = (bpmIndex + 1) % kAvgWindow;
    if (historyCount < kAvgWindow)
        ++historyCount;

    if (historyCount < kAvgWindow)
        return;

    float avgBpm = bpmSum / static_cast<float>(kAvgWindow);
    int roundedBpm = static_cast<int>(avgBpm + 0.5f);

    if (roundedBpm != static_cast<int>(currentBpm)) {
        currentBpm = roundedBpm;
        bpmCallback(currentBpm);
    }
}

} // namespace midi_module
