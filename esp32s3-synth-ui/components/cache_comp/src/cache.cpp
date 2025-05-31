// param_cache.cpp
#include "cache.hpp"
#include "esp_log.h"
static const char *TAG = "ParamCache";

using namespace store;

Cache::Cache(uint8_t voiceCount)
    : data(voiceCount, VoiceCache(VOICE_PAGE_COUNT, PageCache{})),
      globalData{} // zero‐initialize
{
}

int16_t Cache::get(uint8_t voiceIndex, Page page, uint8_t field) const
{
    size_t p = size_t(page);
    if (p >= PAGE_COUNT || field >= MAX_FIELDS)
        return 0;

    if (isGlobal(page))
    {
        return globalData[VOICE_PAGE_COUNT - p][field];
    }
    else
    {
        if (voiceIndex >= data.size())
            return 0;
        return data[voiceIndex][p][field];
    }
}

void Cache::set(const FieldUpdateList &updates)
{
    for (const auto &u : updates)
    {
        size_t p = static_cast<size_t>(u.page);
        if (p >= PAGE_COUNT || u.field >= MAX_FIELDS)
            continue;

        if (isGlobal(u.page))
        {
            globalData[VOICE_PAGE_COUNT - p][u.field] = u.value;
        }
        else
        {
            if (u.voiceIndex >= data.size())
                continue;
            data[u.voiceIndex][p][u.field] = u.value;
        }
    }

    if (callback)
    {
        callback(updates);
    }
}
