#pragma once

#include <vector>
#include "menu_struct.hpp" // for MAX_FIELDS
#include "param_cache.hpp"
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
}
