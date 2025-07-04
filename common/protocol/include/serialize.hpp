#pragma once
#include <cstdint>
#include "esp_log.h"

namespace protocol
{

    inline std::vector<uint8_t> serializeFieldUpdates(const FieldUpdateList &updates)
    {
        const uint8_t count = static_cast<uint8_t>(updates.size());
        std::vector<uint8_t> buffer;
        buffer.reserve(1 + count * sizeof(FieldUpdate));

        buffer.push_back(count);
        const uint8_t *raw = reinterpret_cast<const uint8_t *>(updates.data());
        buffer.insert(buffer.end(),
                      raw,
                      raw + static_cast<size_t>(count) * sizeof(FieldUpdate));
        return buffer;
    }

    inline FieldUpdateList deserializeFieldUpdates(const uint8_t *buffer, size_t length)
    {
        FieldUpdateList result;
        size_t offset = 0;

        while (offset + 1 <= length)
        {
            uint8_t count = buffer[offset];
            size_t packetSize = 1 + static_cast<size_t>(count) * sizeof(FieldUpdate);
            if (offset + packetSize > length)
            {
                ESP_LOGW("PARSER", "truncated/incomplete FieldUpdate packet");
                break;
            }

            auto *updatesStart =
                reinterpret_cast<const FieldUpdate *>(buffer + offset + 1);
            for (size_t i = 0; i < count; ++i)
                result.push_back(updatesStart[i]);

            offset += packetSize;
        }

        return result;
    }

    // –– Helper to build a standalone FieldUpdate packet ––//

    inline std::vector<uint8_t> serializeFieldUpdatePacket(const FieldUpdateList &updates)
    {
        // First, get the “count + payload” bytes
        auto fu = serializeFieldUpdates(updates);

        // Now prefix with the EventType tag
        std::vector<uint8_t> buf;
        buf.reserve(1 + fu.size());
        buf.push_back(static_cast<uint8_t>(EventType::FieldUpdate)); // 0x02
        buf.insert(buf.end(), fu.begin(), fu.end());
        return buf;
    }

    // –– Combined event serialization ––//

    inline std::vector<uint8_t> serializeEvent(const Event &e)
    {
        std::vector<uint8_t> buf;
        buf.push_back(static_cast<uint8_t>(e.type));

        if (e.type == EventType::MidiNote)
        {
            buf.push_back(e.note.status);
            buf.push_back(e.note.note);
            buf.push_back(e.note.velocity);
        }
        else if (e.type == EventType::FieldUpdate)
        {
            auto fu = serializeFieldUpdates(e.fields);
            buf.insert(buf.end(), fu.begin(), fu.end());
        }
        else if (e.type == EventType::BpmFromMidi)
        {
            // Big-endian: high byte first, then low byte
            buf.push_back(static_cast<uint8_t>(e.midiBpm >> 8));
            buf.push_back(static_cast<uint8_t>(e.midiBpm & 0xFF));
        }

        return buf;
    }

    inline std::vector<uint8_t> serializeEvents(const std::vector<Event> &events)
    {
        std::vector<uint8_t> out;
        for (auto &e : events)
        {
            auto pkt = serializeEvent(e);
            out.insert(out.end(), pkt.begin(), pkt.end());
        }
        return out;
    }

    // –– Combined event deserialization ––//

    inline std::vector<Event> deserializeEvents(const uint8_t *buffer, size_t length)
    {
        std::vector<Event> result;
        size_t offset = 0;

        while (offset < length)
        {
            if (offset + 1 > length)
                break;
            EventType type = static_cast<EventType>(buffer[offset++]);
            Event e{type};

            switch (type)
            {
            case EventType::MidiNote:
                if (offset + 3 > length)
                {
                    ESP_LOGW("PARSER", "Incomplete MidiNote packet");
                    return result;
                }
                e.note = MidiNoteEvent{
                    buffer[offset],     // status
                    buffer[offset + 1], // note
                    buffer[offset + 2]  // velocity
                };
                offset += 3;
                break;

            case EventType::FieldUpdate:
            {
                size_t remain = length - offset;
                auto fields = deserializeFieldUpdates(buffer + offset, remain);
                uint8_t count = buffer[offset];
                size_t packetSize = 1 + size_t(count) * sizeof(FieldUpdate);
                offset += (packetSize <= remain ? packetSize : remain);
                e.fields = std::move(fields);
                break;
            }

            case EventType::BpmFromMidi:
                if (offset + 2 > length)
                {
                    ESP_LOGW("PARSER", "Incomplete MidiBpm packet");
                    return result;
                }
                e.midiBpm = (uint16_t(buffer[offset]) << 8) |
                            uint16_t(buffer[offset + 1]);
                offset += 2;
                break;

            default:
                ESP_LOGW("PARSER", "Unknown event type 0x%02X", uint8_t(type));
                return result;
            }

            result.push_back(std::move(e));
        }

        return result;
    }
}
