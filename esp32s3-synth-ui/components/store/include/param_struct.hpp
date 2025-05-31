// parameter_store.hpp
#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <optional>
#include <cinttypes>

namespace store
{

    static constexpr size_t NAME_LEN = 4;
    static inline std::string makeDisplayName(uint8_t index, const std::optional<std::string> &optName)
    {
        if (optName && !optName->empty())
        {
            std::string s = *optName;
            if (s.size() > NAME_LEN)
                s.resize(NAME_LEN);
            else
                s.resize(NAME_LEN, ' ');
            return s;
        }
        char buf[NAME_LEN + 1] = {};
        // zero-padded number width = NAME_LEN
        std::snprintf(buf, sizeof(buf), "%0*hu", (int)NAME_LEN, (unsigned short)(index + 1));
        return std::string(buf);
    }
    /**
     * An entry representing a single voice's parameters.
     */
    struct VoiceStoreEntry
    {
        uint8_t index;                    ///< Slot index (0-based)
        std::optional<std::string> name;  ///< Optional 4-char name
        std::vector<int16_t> voiceParams; ///< Flat parameter array [page][field]
        /**
         * Get the display name: explicit name or fallback "Vn".
         * Always returns a string up to 4 chars.
         */
        inline std::string getName() const
        {
            return makeDisplayName(index, name);
        }
    };

    /**
     * An entry representing a full project: multiple voice entries.
     */
    struct ProjectStoreEntry
    {
        int16_t index;                       ///< Slot index (0-based)
        std::optional<std::string> name;     ///< Optional 4-char name
        std::vector<VoiceStoreEntry> voices; ///< One entry per voice
        std::vector<int16_t> globalParams;   ///< Flat parameter array [page][field]
        /**
         * Get the display name: explicit name or fallback auto-name.
         */
        inline std::string getName() const
        {
            return makeDisplayName(index, name);
        }
    };

    struct NameEntry
    {
        std::string name; ///< The stored name (may be empty)
        bool loaded;      ///< true if this slot was present in NVS
    };

}
