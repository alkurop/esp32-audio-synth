// param_cache.cpp
#include "param_cache.hpp"
#include "esp_log.h"
static const char *TAG = "ParamCache";

namespace menu
{

    ParamCache::ParamCache(uint8_t voiceCount)
        : data(voiceCount, VoiceCache(PageCount - GlobalParamPageCount, PageCache{})),
          globalData{} // zero‐initialize
    {
    }

    int16_t ParamCache::get(uint8_t voiceIndex, Page page, uint8_t field) const
    {
        size_t p = size_t(page);
        if (p >= PageCount || field >= MAX_FIELDS)
            return 0;

        if (isGlobal(page))
        {
            return globalData[p][field];
        }
        else
        {
            if (voiceIndex >= data.size())
                return 0;
            return data[voiceIndex][p][field];
        }
    }

    void ParamCache::set(uint8_t voiceIndex, Page page, uint8_t field, int16_t value)
    {
        // Look up the page and field info
        const auto &pi = menu::menuPages[static_cast<size_t>(page)];
        const auto &fi = pi.fields[field];

        ESP_LOGI(TAG,
                 "Set in cache Voice %u  Page %u (%s)  Field %u (%s) = %d",
                 (unsigned)voiceIndex,
                 (unsigned)page,
                 pi.title,
                 (unsigned)field,
                 fi.label,
                 (int)value);
        size_t p = size_t(page);
        if (p >= PageCount || field >= MAX_FIELDS)
            return;

        if (isGlobal(page))
        {
            globalData[p][field] = value;
        }
        else
        {
            if (voiceIndex >= data.size())
                return;
            data[voiceIndex][p][field] = value;
        }
        if (callback)
        {
            callback(page, field, value);
        }
    }

} // namespace menu
