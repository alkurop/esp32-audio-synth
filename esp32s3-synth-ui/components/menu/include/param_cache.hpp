#pragma once

#include <cstdint>
#include <vector>
#include <array>
#include "menu_struct.hpp"

namespace menu
{

    /**
     * Maximum number of fields per page (knobs 0..3)
     */
    static constexpr uint8_t MAX_FIELDS = 4;

    /**
     * In-memory cache of all voices' parameter values.
     * Dimensions: [voiceCount][Page::_Count][MAX_FIELDS]
     */
    class ParamCache
    {
    public:
        /**
         * @param voiceCount Number of voices (size of cache)
         */
        explicit ParamCache(uint8_t voiceCount);

        /**
         * Read a cached parameter value.
         * @param voiceIndex Zero-based voice index (0..voiceCount-1)
         * @param page       Menu page identifier
         * @param field      Field index on that page (0..MAX_FIELDS-1)
         * @return           Cached value (0 if out-of-range)
         */
        int16_t get(uint8_t voiceIndex, Page page, uint8_t field) const;

        /**
         * Update a cached parameter value.
         * @param voiceIndex Zero-based voice index (0..voiceCount-1)
         * @param page       Menu page identifier
         * @param field      Field index on that page (0..MAX_FIELDS-1)
         * @param value      New value to cache
         */
        void set(uint8_t voiceIndex, Page page, uint8_t field, int16_t value);

        /// Total pages (for convenience)
        static constexpr size_t PageCount = static_cast<size_t>(Page::_Count);

        bool isGlobal(Page page) const
        {
            // only BPM is global for now
            return page == Page::BPM;
        }

    private:
        using PageCache = std::array<uint8_t, MAX_FIELDS>;
        using VoiceCache = std::vector<PageCache>;
        std::vector<VoiceCache> data;
        std::array<PageCache, PageCount> globalData; // [page][field]
    };

} // namespace menu
