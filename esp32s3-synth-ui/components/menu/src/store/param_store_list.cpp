#include <esp_err.h>
#include <nvs_flash.h>
#include <algorithm>
#include <cstdio>
#include <string>
#include <esp_log.h>
#include "param_store.hpp"
#include <inttypes.h>

using namespace menu;
static const char *TAG = "ParamStore";

std::vector<NameEntry> ParamStore::listProjectNames()
{
    std::lock_guard<std::mutex> lock(nvsMutex);

    std::vector<NameEntry> entries(maxProjects);

    // Try to open NVS once
    nvs_handle handle;
    if (nvs_open(NVS_NAMESPACE, NVS_READONLY, &handle) != ESP_OK)
    {
        // on error, return default entries (all unloaded, auto-named)
        for (uint8_t i = 0; i < maxProjects; ++i)
        {
            entries[i].name = makeDisplayName(i, std::nullopt);
            entries[i].loaded = false;
        }
        return entries;
    }

    // For each project slot
    for (uint8_t i = 0; i < maxProjects; ++i)
    {
        // default display name (e.g. "P1", "P2", …) and unloaded
        entries[i].name = makeDisplayName(i, std::nullopt);
        entries[i].loaded = false;

        // key = "proj_name_<i>"
        char key[32];
        std::snprintf(key, sizeof(key), "%s%u", KEY_PROJ_NAME, i);

        // probe length
        size_t len = 0;
        if (nvs_get_str(handle, key, nullptr, &len) == ESP_OK && len > 0)
        {
            // read the actual name
            std::string nm(len, '\0');
            nvs_get_str(handle, key, &nm[0], &len);
            if (!nm.empty())
            {
                entries[i].name = nm;
                entries[i].loaded = true;
            }
        }
    }

    nvs_close(handle);
    return entries;
}

std::vector<NameEntry> ParamStore::listVoiceNames()
{
    std::lock_guard<std::mutex> lock(nvsMutex);

    std::vector<NameEntry> entries(maxVoices);
    for (uint8_t i = 0; i < maxVoices; ++i)
    {
        entries[i].name = makeDisplayName(i, std::nullopt);
        entries[i].loaded = false;
    }

    nvs_handle handle;
    if (nvs_open(NVS_NAMESPACE, NVS_READONLY, &handle) != ESP_OK)
        return entries;

    char key[32];
    for (uint8_t i = 0; i < maxVoices; ++i)
    {
        size_t len = 0;
        std::snprintf(key, sizeof(key), "%s%u", KEY_VOICE_NAME, i);
        if (nvs_get_str(handle, key, nullptr, &len) == ESP_OK && len > 1)
        {
            std::string name(len, '\0');
            nvs_get_str(handle, key, &name[0], &len);
            entries[i].name = name;
            entries[i].loaded = true;
        }
    }

    nvs_close(handle);
    return entries;
}
