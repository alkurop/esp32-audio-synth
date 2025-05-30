#include <esp_err.h>
#include <nvs_flash.h>
#include <algorithm>
#include <cstdio>
#include <string>
#include <esp_log.h>
#include "param_store.hpp"
#include "param_store.hpp"
#include <inttypes.h>
using namespace menu;
static constexpr char KEY_GLOB_FIELD[] = "gf_";
static const char *TAG = "ParamStore";

ParamStore::ParamStore(uint8_t maxProjects, uint8_t maxVoices) : maxProjects(maxProjects), maxVoices(maxVoices)
{
    nvs_flash_init();
}

std::vector<int16_t> ParamStore::getGlobalFields()
{
    std::lock_guard<std::mutex> lock(nvsMutex);

    std::vector<int16_t> fields;
    nvs_handle handle;
    if (nvs_open(NVS_NAMESPACE, NVS_READONLY, &handle) != ESP_OK)
        return fields;

    size_t pageCount = static_cast<size_t>(GlobalField::_Count);
    for (size_t p = 0; p < pageCount; ++p)
    {
        for (uint8_t f = 0; f < MAX_FIELDS; ++f)
        {
            char key[32];
            std::snprintf(key, sizeof(key), "%s%u_%u", KEY_GLOB_FIELD, p, f);
            int16_t v = 0;
            nvs_get_i16(handle, key, &v);
            fields.push_back(v);
        }
    }
    nvs_close(handle);
    return fields;
}

void ParamStore::saveGlobalField(Page page, uint8_t field, int16_t value)
{
    std::lock_guard<std::mutex> lock(nvsMutex);

    nvs_handle handle;
    if (nvs_open(NVS_NAMESPACE, NVS_READWRITE, &handle) != ESP_OK)
        return;
    char key[32];
    std::snprintf(key, sizeof(key), "%s%u_%u", KEY_GLOB_FIELD, static_cast<unsigned>(page), field);
    nvs_set_i16(handle, key, value);
    nvs_commit(handle);
    nvs_close(handle);
}
