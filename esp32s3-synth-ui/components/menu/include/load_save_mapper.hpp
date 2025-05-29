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
}
