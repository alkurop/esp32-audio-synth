#include <esp_err.h>
#include <nvs_flash.h>
#include <algorithm>
#include <cstdio>
#include <string>
#include <esp_log.h>
#include "param_store.hpp"
#include <inttypes.h>

using namespace menu;
static constexpr char KEY_PROJ_DATA[] = "pd_";
static constexpr char KEY_PROJ_VOICE_COUNT[] = "pdc_"; // + project slot
static constexpr char KEY_PROJ_VOICE_NAME[] = "pvn_";  // + project slot + "_" + voice index

static const char *TAG = "ParamStore";

void ParamStore::saveProject(const ProjectStoreEntry &entry, bool allowAutosave)
{
    std::unique_lock<std::mutex> lock(nvsMutex);

    if (entry.index >= maxProjects)
    {
        ESP_LOGW(TAG, "saveProject: slot %d out of range", entry.index);
        return;
    }

    nvs_handle h;
    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &h);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "saveProject: nvs_open failed (%d)", err);
        return;
    }

    char key[32];
    esp_err_t rc;

    // Save the project’s display name using getName()
    std::snprintf(key, sizeof(key), "%s%d", KEY_PROJ_NAME, entry.index);
    std::string name = entry.getName();
    rc = nvs_set_str(h, key, name.c_str());
    if (rc != ESP_OK)
        ESP_LOGE(TAG, "nvs_set_str(%s) failed (%d)", key, rc);

    // Flatten voices params into blob
    const size_t P = VOICE_PAGE_COUNT;
    const size_t F = MAX_FIELDS;
    std::vector<int16_t> blob;
    blob.reserve(entry.voices.size() * P * F);
    for (const auto &ve : entry.voices)
    {
        if (ve.params.size() != P * F)
        {
            ESP_LOGW(TAG, "voice %d has %zu params, expected %zu", ve.index, ve.params.size(), P * F);
        }
        blob.insert(blob.end(), ve.params.begin(), ve.params.end());
    }

    // Save blob data
    std::snprintf(key, sizeof(key), "%s%d", KEY_PROJ_DATA, entry.index);
    rc = nvs_set_blob(h, key, blob.data(), blob.size() * sizeof(blob[0]));
    if (rc != ESP_OK)
        ESP_LOGE(TAG, "nvs_set_blob(%s) failed (%d)", key, rc);

    // Save voice count
    std::snprintf(key, sizeof(key), "%s%d", KEY_PROJ_VOICE_COUNT, entry.index);
    rc = nvs_set_i32(h, key, static_cast<int32_t>(entry.voices.size()));
    if (rc != ESP_OK)
        ESP_LOGE(TAG, "nvs_set_i32(%s) failed (%d)", key, rc);

    rc = nvs_commit(h);
    if (rc != ESP_OK)
        ESP_LOGE(TAG, "nvs_commit failed (%d)", rc);

    nvs_close(h);

    // Explicit autosave logic without deadlock
    if (allowAutosave && entry.index != AUTOSAVE_SLOT)
    {
        ProjectStoreEntry autoEntry = entry;
        autoEntry.index = AUTOSAVE_SLOT;
        lock.unlock(); // explicitly unlock mutex before recursive call
        saveProject(autoEntry, false);
        ESP_LOGD(TAG, "Autosaved project to slot %d", AUTOSAVE_SLOT);
    }
}

ProjectStoreEntry ParamStore::loadProject(int16_t index)
{
    std::unique_lock<std::mutex> lock(nvsMutex);

    ProjectStoreEntry entry{};
    entry.index = index;

    // 0) Range check
    if (index >= maxProjects)
    {
        ESP_LOGW(TAG, "loadProject: slot %d out of range", index);
        return entry;
    }

    // 1) Open NVS
    nvs_handle handle;
    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READONLY, &handle);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "loadProject: nvs_open failed (%d)", static_cast<int>(err));
        return entry;
    }

    char key[32];

    // 2) Load project name
    std::snprintf(key, sizeof(key), "%s%d", KEY_PROJ_NAME, entry.index);
    ESP_LOGD(TAG, "loadProject: reading nameKey=\"%s\"", key);
    size_t nameLen = 0;
    err = nvs_get_str(handle, key, nullptr, &nameLen);
    if (err == ESP_OK && nameLen > 1)
    {
        std::string projName(nameLen, '\0');
        nvs_get_str(handle, key, &projName[0], &nameLen);
        entry.name = projName;
        ESP_LOGD(TAG, "  Project name=\"%s\"", projName.c_str());
    }
    else
    {
        ESP_LOGD(TAG, "  No project name stored (err=%d)", static_cast<int>(err));
    }

    // 3) Read how many voices
    int32_t count = 0;
    char cntKey[32];
    std::snprintf(cntKey, sizeof(cntKey), "%s%d", KEY_PROJ_VOICE_COUNT, entry.index);
    err = nvs_get_i32(handle, cntKey, &count);
    ESP_LOGD(TAG, "loadProject: voiceCountKey=\"%s\", count=%d, err=%d", cntKey, static_cast<int>(count), static_cast<int>(err));
    if (err != ESP_OK || count <= 0)
    {
        ESP_LOGW(TAG, "  No voices to load (count=%d)", static_cast<int>(count));
        nvs_close(handle);
        return entry;
    }

    // 4) Probe & read the big blob
    const size_t P = VOICE_PAGE_COUNT;
    const size_t F = MAX_FIELDS;
    const size_t voiceSize = P * F;
    size_t expectedBytes = static_cast<size_t>(count) * voiceSize * sizeof(int16_t);

    std::snprintf(key, sizeof(key), "%s%d", KEY_PROJ_DATA, entry.index);
    ESP_LOGD(TAG, "loadProject: dataKey=\"%s\", expecting %zu bytes", key, expectedBytes);

    size_t blobLen = 0;
    err = nvs_get_blob(handle, key, nullptr, &blobLen);
    if (err != ESP_OK || blobLen == 0)
    {
        ESP_LOGE(TAG, "  nvs_get_blob probe failed (err=%d) or zero length", static_cast<int>(err));
        nvs_close(handle);
        return entry;
    }
    ESP_LOGD(TAG, "  Blob length = %zu bytes", blobLen);

    // Read into flat vector
    std::vector<int16_t> blob(blobLen / sizeof(int16_t));
    err = nvs_get_blob(handle, key, blob.data(), &blobLen);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "  nvs_get_blob read failed (err=%d)", static_cast<int>(err));
        nvs_close(handle);
        return entry;
    }
    if (blobLen != expectedBytes)
    {
        ESP_LOGW(TAG, "  blobLen (%zu) != expected (%zu)", blobLen, expectedBytes);
        nvs_close(handle);
        return entry; // <- Fast return point here
    }

    // 5) Split into voices, padding each out to P*F entries
    entry.voices.clear();
    entry.voices.reserve(count);

    for (int32_t v = 0; v < count; ++v)
    {
        VoiceStoreEntry ve{};
        ve.index = static_cast<uint8_t>(v);

        size_t offset = static_cast<size_t>(v) * voiceSize;
        size_t avail = (blob.size() >= offset + voiceSize) ? voiceSize : blob.size() - offset;
        // copy available data...
        ve.params.assign(
            blob.begin() + offset,
            blob.begin() + offset + avail);

        // ...and pad with zeros up to full voiceSize
        if (avail < voiceSize)
        {
            ve.params.resize(voiceSize, 0);
        }

        entry.voices.push_back(std::move(ve));
    }

    nvs_close(handle);
    ESP_LOGD(TAG, "loadProject: done, loaded %zu voices", entry.voices.size());
    return entry;
}
