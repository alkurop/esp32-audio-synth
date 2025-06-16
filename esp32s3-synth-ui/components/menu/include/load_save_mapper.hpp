#pragma once

#include <vector>
#include "menu_struct.hpp" // for MAX_FIELDS
#include "param_struct.hpp"

using namespace protocol;
using namespace store;

namespace menu
{

    /// Flatten a VoiceCache into a single vector<int16_t>
    inline std::vector<int16_t> flattenVoiceParams(const VoiceCache &vc)
    {
        std::vector<int16_t> flat;
        flat.reserve(vc.size() * MAX_FIELDS);
        for (const auto &page : vc)
        {
            for (int16_t v : page)
            {
                flat.push_back(v);
            }
        }
        return flat;
    }

    /// Reverse: take a flat vector<int16_t> and rebuild a VoiceCache
    inline VoiceCache unflattenVoiceParams(const std::vector<int16_t> &flat)
    {
        // Determine how many pages we have
        size_t pageCount = flat.size() / MAX_FIELDS;
        VoiceCache vc(pageCount);

        for (size_t p = 0; p < pageCount; ++p)
        {
            for (size_t f = 0; f < MAX_FIELDS; ++f)
            {
                // Copy back into the signed PageCache
                vc[p][f] = flat[p * MAX_FIELDS + f];
            }
        }

        return vc;
    }

    inline std::vector<int16_t> flattenGlobalParams(const GlobalCache &globalData)
    {
        std::vector<int16_t> flat;
        flat.reserve(GLOBAL_PAGE_COUNT * MAX_FIELDS);

        for (const auto &page : globalData)
        {
            for (int16_t v : page)
            {
                flat.push_back(v);
            }
        }

        return flat;
    }

    inline std::array<PageCache, GLOBAL_PAGE_COUNT> unflattenGlobalParams(const std::vector<int16_t> &flat)
    {
        std::array<PageCache, GLOBAL_PAGE_COUNT> globalData{};

        for (size_t p = 0; p < GLOBAL_PAGE_COUNT; ++p)
        {
            for (size_t f = 0; f < MAX_FIELDS; ++f)
            {
                globalData[p][f] = flat[p * MAX_FIELDS + f];
            }
        }

        return globalData;
    }

    FieldUpdateList mapVoiceStoreEntryToUpdates(const VoiceStoreEntry &ve)
    {
        FieldUpdateList updates;

        const auto &flat = ve.voiceParams;
        size_t pageCount = flat.size() / MAX_FIELDS;

        for (size_t pageIndex = 0; pageIndex < pageCount; ++pageIndex)
        {
            const auto &pageItem = menuPages[pageIndex];
            size_t fields = pageItem.fieldCount;

            for (size_t field = 0; field < fields; ++field)
            {
                size_t idx = pageIndex * MAX_FIELDS + field;
                if (idx >= flat.size())
                    break;

                int16_t value = flat[idx];
                updates.push_back(FieldUpdate{
                    ve.index,
                    static_cast<uint8_t>(pageIndex),
                    static_cast<uint8_t>(field),
                    value});
            }
        }

        return updates;
    }

    FieldUpdateList mapProjectEntryToUpdates(const ProjectStoreEntry &projectEntry)
    {
        FieldUpdateList updates;

        // Voice parameter updates
        for (const auto &ve : projectEntry.voices)
        {
            FieldUpdateList voiceUpdates = mapVoiceStoreEntryToUpdates(ve);
            updates.insert(updates.end(), voiceUpdates.begin(), voiceUpdates.end());
        }

        // Global parameter updates
        const auto &flatGlobal = projectEntry.globalParams;
        if (!flatGlobal.empty())
        {
            for (size_t p = 0; p < GLOBAL_PAGE_COUNT; ++p)
            {
                size_t pageIndex = VOICE_PAGE_COUNT + p;
                if (pageIndex >= PAGE_COUNT)
                    break;

                const auto &pi = menuPages[pageIndex];
                size_t fields = std::min<size_t>(pi.fieldCount, MAX_FIELDS);

                for (size_t f = 0; f < fields; ++f)
                {
                    size_t idx = p * MAX_FIELDS + f;
                    if (idx >= flatGlobal.size())
                        break;

                    int16_t value = flatGlobal[idx];
                    updates.push_back(FieldUpdate{
                        static_cast<uint8_t>(-1),
                        static_cast<uint8_t>(pageIndex),
                        static_cast<uint8_t>(f),
                        value});
                }
            }
        }

        return updates;
    }

    struct ChannelPage
    {
        uint8_t channel = 0;
        uint8_t volume = 0;
    };

    std::optional<ChannelPage> parseChannelPage(const std::vector<int16_t> &flat)
    {
        size_t base = static_cast<size_t>(Page::Channel) * MAX_FIELDS;
        if (base + std::max(
                       static_cast<size_t>(ChannelField::Chan),
                       static_cast<size_t>(ChannelField::Vol)) >=
            flat.size())
            return std::nullopt;

        return ChannelPage{
            .channel = static_cast<uint8_t>(flat[base + static_cast<size_t>(ChannelField::Chan)]),
            .volume = static_cast<uint8_t>(flat[base + static_cast<size_t>(ChannelField::Vol)])};
    }
}
