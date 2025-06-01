#pragma once
#include <cstdint>
#include <vector>
#include <cstring>

#include "menu_struct.hpp"

namespace protocol
{
    using FieldUpdateList = std::vector<FieldUpdate>;

    inline std::vector<uint8_t> serializeFieldUpdates(const FieldUpdateList &updates)
    {
        const uint8_t count = updates.size();
        std::vector<uint8_t> buffer;
        buffer.reserve(1 + count * sizeof(FieldUpdate));

        buffer.push_back(count); // Prefix with count

        const uint8_t *raw = reinterpret_cast<const uint8_t *>(updates.data());
        buffer.insert(buffer.end(), raw, raw + count * sizeof(FieldUpdate));

        return buffer;
    }

    inline FieldUpdateList deserializeFieldUpdates(const uint8_t *buffer, size_t length)
    {
        FieldUpdateList result;

        if (length < 1)
            return result; // no count byte

        uint8_t count = buffer[0];
        size_t expected = 1 + count * sizeof(FieldUpdate);
        if (length < expected)
            return result; // not enough data

        result.resize(count);
        std::memcpy(result.data(), buffer + 1, count * sizeof(FieldUpdate));
        return result;
    }

}
