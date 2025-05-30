#include <esp_err.h>
#include <nvs_flash.h>
#include <algorithm>
#include <cstdio>
#include <string>
#include <esp_log.h>
#include "param_store.hpp"
#include <inttypes.h>

using namespace menu;
static constexpr char KEY_VOICE_DATA[] = "vd_";
static const char *TAG = "ParamStore";

VoiceStoreEntry ParamStore::loadVoice(int16_t index)
{
    std::lock_guard<std::mutex> lock(nvsMutex);

    // 1) Prepare an empty entry
    VoiceStoreEntry entry{.index = index, .name = std::nullopt, .params = {}};

    if (index >= maxVoices)
    {
        ESP_LOGW(TAG, "loadVoice: slot %d out of range", index);
        return entry;
    }

    // 2) Open NVS for read
    nvs_handle handle;
    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READONLY, &handle);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "nvs_open RO failed (%d)", err);
        return entry;
    }

    // 3) Load the optional name key & value
    {
        char nameKey[32];
        std::snprintf(nameKey, sizeof(nameKey), "%s%d", KEY_VOICE_NAME, index);
        ESP_LOGD(TAG, "loadVoice: nameKey='%s'", nameKey);

        size_t len = 0;
        err = nvs_get_str(handle, nameKey, nullptr, &len);
        if (err == ESP_OK && len > 1)
        {
            std::string name(len, '\0');
            nvs_get_str(handle, nameKey, &name[0], &len);
            entry.name = name;
        }
    }

    // 4) Load the params blob key & data
    {
        char dataKey[32];
        std::snprintf(dataKey, sizeof(dataKey), "%s%d", KEY_VOICE_DATA, index);

        // Probe the blob length
        size_t blobLen = 0;
        err = nvs_get_blob(handle, dataKey, nullptr, &blobLen);
        if (err == ESP_OK && blobLen > 0)
        {
            const size_t P = VOICE_PAGE_COUNT;
            const size_t F = MAX_FIELDS;
            const size_t voiceSize = P * F;
            size_t expectedBytes = voiceSize * sizeof(int16_t);
            size_t count = blobLen / sizeof(int16_t);

            if (count != expectedBytes)
            {
                ESP_LOGW(TAG, "Invalid param count (%u) for slot %u, expected %u. Aborting load.",
                         (unsigned)count, (unsigned)index, (unsigned)expectedBytes);
                nvs_close(handle);
                return entry; // <- Fast return point here
            }
            ESP_LOGD(TAG, "  blobLen=%u bytes => %u int16_t entries",
                     (unsigned)blobLen, (unsigned)count);

            entry.params.resize(count);
            err = nvs_get_blob(handle, dataKey, entry.params.data(), &blobLen);
            if (err != ESP_OK)
            {
                ESP_LOGE(TAG, "  nvs_get_blob(%s) failed (%d)", dataKey, err);
                entry.params.clear();
            }
        }
        else
        {
            ESP_LOGW(TAG, "  No params blob for slot %u (err %d)", index, err);
        }
    }

    nvs_close(handle);
    return entry;
}

void ParamStore::saveVoice(const VoiceStoreEntry &entry)
{
    std::lock_guard<std::mutex> lock(nvsMutex);

    // 1) Open NVS for read/write
    nvs_handle handle;
    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &handle);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "saveVoice: nvs_open RW failed (%d)", err);
        return;
    }

    auto slot = entry.index;
    if (slot >= maxVoices)
    {
        ESP_LOGW(TAG, "saveVoice: slot %u out of range", slot);
        nvs_close(handle);
        return;
    }

    char key[32];
    esp_err_t rc;

    // --- save the name ---
    std::snprintf(key, sizeof(key), "%s%d", KEY_VOICE_NAME, slot);
    ESP_LOGD(TAG, "saveVoice: nameKey='%s', name='%s'", key, entry.name ? entry.name->c_str() : "<empty>");
    rc = nvs_set_str(handle, key, entry.name ? entry.name->c_str() : "");
    if (rc != ESP_OK)
    {
        ESP_LOGE(TAG, "  nvs_set_str(%s) failed (%d)", key, rc);
    }

    // --- save the params blob ---
    std::snprintf(key, sizeof(key), "%s%d", KEY_VOICE_DATA, slot);
    size_t blobSize = entry.params.size() * sizeof(entry.params[0]);
    ESP_LOGD(TAG, "saveVoice: dataKey='%s', blobSize=%zu bytes (%zu entries)", key, blobSize, entry.params.size());
    rc = nvs_set_blob(handle, key, entry.params.data(), blobSize);
    if (rc != ESP_OK)
    {
        ESP_LOGE(TAG, "  nvs_set_blob(%s) failed (%d)", key, rc);
    }

    // --- commit and close ---
    rc = nvs_commit(handle);
    if (rc != ESP_OK)
    {
        ESP_LOGE(TAG, "  nvs_commit failed (%d)", rc);
    }
    nvs_close(handle);
}
