namespace protocol
{
    using FieldUpdateList = std::vector<FieldUpdate>;

    inline std::vector<uint8_t> serializeFieldUpdates(const FieldUpdateList &updates)
    {
        // 1 byte for “count” + N * 5 bytes per FieldUpdate
        const uint8_t count = static_cast<uint8_t>(updates.size());
        std::vector<uint8_t> buffer;
        buffer.reserve(1 + count * sizeof(FieldUpdate));

        // Prefix with the count
        buffer.push_back(count);

        // Append raw bytes of each FieldUpdate in sequence
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

        // Parse “count + payload” repeatedly as long as there’s enough data
        while (offset + 1 <= length)
        {
            // Read the ‘count’ byte
            uint8_t count = buffer[offset];
            // Total bytes for this packet = 1 + count * sizeof(FieldUpdate)
            size_t packetSize = 1 + static_cast<size_t>(count) * sizeof(FieldUpdate);

            // If the buffer is too short for this packet, bail out
            if (offset + packetSize > length)
            {
                ESP_LOGW("PARSER", "truncated/incomplete packet → stop parsing");
                break;
            }

            // Interpret the next `count` FieldUpdates directly
            auto *updatesStart =
                reinterpret_cast<const FieldUpdate *>(buffer + offset + 1);
            for (size_t i = 0; i < count; i++)
            {
                result.push_back(updatesStart[i]);
            }

            // Advance to the next packet (if any)
            offset += packetSize;
        }

        return result;
    }
}
