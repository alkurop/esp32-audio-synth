#pragma once

#include <cstdint>
#include <functional>

namespace midi_module
{
    class BpmCounter
    {
    public:
        using BpmCallback = std::function<void(uint16_t bpm)>;

        BpmCounter();

        void setCallback(BpmCallback callback);
        void onClockTick(uint64_t timestamp_us); // call this on each 0xF8 clock tick
        void stop();
        void start();

    private:
        BpmCallback bpmCallback;
        uint64_t lastBeatTimestamp = 0;
        uint8_t clockCount = 0;
        uint16_t currentBpm = 0;

        static constexpr uint8_t clocksPerBeat = 24;
        bool isActive = true;
        uint8_t historyCount = 0; // number of BPM samples collected so far

        // Moving average smoothing
        static constexpr size_t kAvgWindow = 4;
        float bpmHistory[kAvgWindow] = {0.0f};
        float bpmSum = 0.0f;
        uint8_t bpmIndex = 0;
    };

} // namespace midi_module
