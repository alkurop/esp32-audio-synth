#pragma once
#include <cstdint>

namespace midi_module

{
    struct ControllerChange
    {
        uint8_t channel;    // MIDI channel (0–15)
        uint8_t controller; // Controller number (e.g. 1 = mod wheel)
        uint8_t value;      // Value (0–127)
    };

    struct SongPosition
    {
        uint16_t position; // Position in MIDI beats (16th notes)
    };

    struct MidiNoteEvent
    {
        uint8_t status;   // e.g. 0x9 = Note-On, 0x8 = Note-Off
        uint8_t note;     // 0–127
        uint8_t velocity; // 0–127
        bool isNoteOn() const
        {
            // 0x90–0x9F = Note-On for channels 0–15
            // treat velocity == 0 as “off” per MIDI spec
            return ((status & 0xF0) == 0x90) && (velocity > 0);
        }

        bool isNoteOff() const
        {
            // 0x80–0x8F = Note-Off OR Note-On with zero velocity
            uint8_t type = status & 0xF0;
            return (type == 0x80) ||
                   ((type == 0x90) && (velocity == 0));
        }
        uint8_t channel() const
        {
            return status & 0x0F;
        }
    };

    enum class TransportCommand : uint8_t
    {
        Start = 0xFA,
        Continue = 0xFB,
        Stop = 0xFC,
        Unknown = 0x00
    };

    struct TransportEvent
    {
        TransportCommand command;
    };

    inline MidiNoteEvent parseMidiNoteEvent(const uint8_t packet[4])
    {
        // packet[0] is the 0x01 tag—skip it
        MidiNoteEvent e;
        e.status = packet[1]; // high nibble: 8 or 9
        e.note = packet[2];
        e.velocity = packet[3];
        return e;
    }

} // namespace protocol
