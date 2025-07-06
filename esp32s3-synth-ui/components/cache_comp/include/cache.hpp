#pragma once

#include <cstdint>
#include <vector>
#include <array>
#include "menu_struct.hpp"
#include "protocol.hpp"
#include <functional>

using namespace protocol;

namespace store
{
    /**
     * Maximum number of fields per page (knobs 0..3)
     */
    using PageCache = std::array<int16_t, MAX_FIELDS>;
    using VoiceCache = std::vector<PageCache>;
    using GlobalCache = std::array<PageCache, GLOBAL_PAGE_COUNT>;

    class Cache
    {
    public:
        explicit Cache(uint8_t voiceCount);

        int16_t get(uint8_t voiceIndex, Page page, uint8_t field) const;

        void set(const FieldUpdateList &updates);

        bool isGlobal(Page page) const
        {
            // only BPM is global for now
            return page == Page::Bpm;
        }

        const std::vector<VoiceCache> &getVoiceData() const { return data; }
        const GlobalCache &getGlobalCache() const { return globalData; }
        void setCallback(FieldUpdateCallback callback) { this->callback = std::move(callback); }

    private:
        std::vector<VoiceCache> data;
        GlobalCache globalData;
        FieldUpdateCallback callback;
    };

} // namespace menu
