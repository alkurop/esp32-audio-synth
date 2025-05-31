#pragma once
#include <cstdint>

// Voice index 0…15 → bits 4–7; parameter index 0…15 → bits 0–3
// paramID = (voiceIndex << 4) | fieldIndex
// That gives you 16 voices × 16 fields = 256 total slots.

// then a list of fieldIndices (matching your menu fields):

namespace protocol
{
    enum Param : uint8_t
    {
        // page 0 = Channel (Chan, Vol, BPM)
        CHAN = 0x00, // word value = 1…16
        VOL = 0x01,  // 0…32
        BPM = 0x02,  // 30…300

        // page 1 = Oscillator (Shape, Morph, PWM, Sync)
        SHAPE = 0x10,
        MORPH = 0x11,
        PWM = 0x12,
        SYNC = 0x13,

        // page 2 = Filter (Type, Cut, Res)
        FILTER_TYPE = 0x20,
        FILTER_CUT = 0x21,
        FILTER_RES = 0x22,

        // page 3 = Envelope (A,D,S,R)
        ENV_A = 0x30,
        ENV_D = 0x31,
        ENV_S = 0x32,
        ENV_R = 0x33,

        // page 4 = Tuning (Octave, Semitone, FineTune)
        TUNE_OCTAVE = 0x40,
        TUNE_SEMITONE = 0x41,
        TUNE_FINETUNE = 0x42,

        // page 5 = Filter LFO (RateMode, Rate, Subdiv, Depth)
        LFO1_MODE = 0x50,
        LFO1_RATE = 0x51,
        LFO1_SUB = 0x52,
        LFO1_DEPTH = 0x53,

        // page 6 = Detune LFO (RateMode, Rate, Subdiv, Depth)
        LFO2_MODE = 0x60,
        LFO2_RATE = 0x61,
        LFO2_SUB = 0x62,
        LFO2_DEPTH = 0x63,
    };
}
