#include <esp_err.h>
#include <nvs_flash.h>
#include <algorithm>
#include <cstdio>
#include <string>
#include <esp_log.h>
#include "param_store.hpp"
#include <inttypes.h>

using namespace menu;
static constexpr char NVS_NAMESPACE[] = "synth";
static constexpr char KEY_PROJ_COUNT[] = "pc";
static constexpr char KEY_PROJ_NAME[] = "pn_";
static constexpr char KEY_PROJ_DATA[] = "pd_";
static constexpr char KEY_GLOB_FIELD[] = "gf_";
static constexpr char KEY_VOICE_NAME[] = "vn_";
static constexpr char KEY_VOICE_DATA[] = "vd_";
static constexpr char KEY_PROJ_VOICE_COUNT[] = "pdc_"; // + project slot
static constexpr char KEY_PROJ_VOICE_NAME[] = "pvn_";  // + project slot + "_" + voice index

static constexpr uint8_t MAX_FIELDS = 4;
static const char *TAG = "ParamStore";

ParamStore::ParamStore(uint8_t maxProjects, uint8_t maxVoices) : maxProjects(maxProjects), maxVoices(maxVoices)
{
    nvs_flash_init();
}

void ParamStore::saveProject(const ProjectStoreEntry &entry)
{
    // 0) Sanity check slot
    if (entry.index >= maxProjects)
    {
        ESP_LOGW(TAG, "saveProject: slot %u out of range", entry.index);
        return;
    }

    // 1) Open NVS for read/write
    nvs_handle h;
    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &h);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "saveProject: nvs_open failed (%d)", err);
        return;
    }

    char key[32];
    esp_err_t rc;

    // 2) Save the project’s display name
    std::snprintf(key, sizeof(key), "%s%u", KEY_PROJ_NAME, entry.index);
    ESP_LOGD(TAG, "saveProject: nameKey=\"%s\", name=\"%s\"", key, entry.name.has_value() ? entry.name->c_str() : "<none>");
    rc = nvs_set_str(h, key, entry.name.has_value() ? entry.name->c_str() : "");
    if (rc != ESP_OK)
    {
        ESP_LOGE(TAG, "  nvs_set_str(%s) failed (%d)", key, rc);
    }

    // 3) Update the global project count
    int32_t oldCount = 0;
    nvs_get_i32(h, KEY_PROJ_COUNT, &oldCount); // ignore errors: treat missing as 0
    int32_t newCount = std::max<int32_t>(oldCount, entry.index + 1);
    ESP_LOGD(TAG, "saveProject: updating %s from %d to %d", KEY_PROJ_COUNT, static_cast<int>(oldCount), static_cast<int>(newCount));
    rc = nvs_set_i32(h, KEY_PROJ_COUNT, newCount);
    if (rc != ESP_OK)
    {
        ESP_LOGE(TAG, "  nvs_set_i32(%s) failed (%d)", KEY_PROJ_COUNT, rc);
    }

    // 4) Flatten all voices' params into one big blob
    const size_t P = VOICE_PAGE_COUNT;
    const size_t F = MAX_FIELDS;
    std::vector<int16_t> blob;
    blob.reserve(entry.voices.size() * P * F);
    for (const auto &ve : entry.voices)
    {
        if (ve.params.size() != P * F)
        {
            ESP_LOGW(TAG,
                     "  voice %u has %zu params, expected %zu",
                     ve.index,
                     ve.params.size(),
                     P * F);
        }
        blob.insert(blob.end(), ve.params.begin(), ve.params.end());
    }

    // 5) Save the flattened blob
    std::snprintf(key, sizeof(key), "%s%u", KEY_PROJ_DATA, entry.index);
    ESP_LOGD(TAG,
             "saveProject: dataKey=\"%s\", blobSize=%zu bytes (%zu voices)",
             key,
             blob.size() * sizeof(blob[0]),
             entry.voices.size());
    rc = nvs_set_blob(
        h,
        key,
        blob.data(),
        blob.size() * sizeof(blob[0]));
    if (rc != ESP_OK)
    {
        ESP_LOGE(TAG, "  nvs_set_blob(%s) failed (%d)", key, rc);
    }

    // 6) Save the number of voices in this project
    std::snprintf(key, sizeof(key), "%s%u", KEY_PROJ_VOICE_COUNT, entry.index);
    ESP_LOGD(TAG, "saveProject: cntKey=\"%s\", count=%zu",
             key,
             entry.voices.size());
    rc = nvs_set_i32(h, key, static_cast<int32_t>(entry.voices.size()));
    if (rc != ESP_OK)
    {
        ESP_LOGE(TAG, "  nvs_set_i32(%s) failed (%d)", key, rc);
    }

    // 7) Commit and close
    rc = nvs_commit(h);
    if (rc != ESP_OK)
    {
        ESP_LOGE(TAG, "  nvs_commit failed (%d)", rc);
    }
    nvs_close(h);

    // 8) Remember the current project index
    currentProjectIndex = entry.index;
}

ProjectStoreEntry ParamStore::loadProject(uint8_t index)
{
    ProjectStoreEntry entry{};
    entry.index = index;

    // 0) Range check
    if (index >= maxProjects)
    {
        ESP_LOGW(TAG, "loadProject: slot %u out of range", index);
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
    std::snprintf(key, sizeof(key), "%s%u", KEY_PROJ_NAME, entry.index);
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
    std::snprintf(cntKey, sizeof(cntKey), "%s%u", KEY_PROJ_VOICE_COUNT, entry.index);
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

    std::snprintf(key, sizeof(key), "%s%u", KEY_PROJ_DATA, entry.index);
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
        ESP_LOGW(TAG,
                 "  blobLen (%zu) != expected (%zu)",
                 blobLen, expectedBytes);
    }

    // 5) Split into voices, padding each out to P*F entries
    entry.voices.clear();
    entry.voices.reserve(count);

    for (int32_t v = 0; v < count; ++v)
    {
        VoiceStoreEntry ve{};
        ve.index = static_cast<uint8_t>(v);

        size_t offset = static_cast<size_t>(v) * voiceSize;
        size_t avail = (blob.size() >= offset + voiceSize)
                           ? voiceSize
                           : blob.size() - offset;

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
    currentProjectIndex = entry.index;
    ESP_LOGD(TAG, "loadProject: done, loaded %zu voices", entry.voices.size());
    return entry;
}

uint8_t ParamStore::getCurrentProjectIndex() const
{
    return currentProjectIndex;
}

std::vector<int16_t> ParamStore::getGlobalFields() const
{
    std::vector<int16_t> fields;
    if (currentProjectIndex >= maxProjects)
        return fields;
    nvs_handle handle;
    if (nvs_open(NVS_NAMESPACE, NVS_READONLY, &handle) != ESP_OK)
        return fields;

    size_t pageCount = static_cast<size_t>(Page::_Count);
    for (size_t p = 0; p < pageCount; ++p)
    {
        for (uint8_t f = 0; f < MAX_FIELDS; ++f)
        {
            char key[32];
            std::snprintf(key, sizeof(key), "%s%u_%zu_%u", KEY_GLOB_FIELD, (unsigned)currentProjectIndex, p, f);
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
    if (currentProjectIndex >= maxProjects)
        return;
    nvs_handle handle;
    if (nvs_open(NVS_NAMESPACE, NVS_READWRITE, &handle) != ESP_OK)
        return;
    char key[32];
    std::snprintf(key, sizeof(key), "%s%u_%u_%u", KEY_GLOB_FIELD, (unsigned)currentProjectIndex, static_cast<unsigned>(page), field);
    nvs_set_i16(handle, key, value);
    nvs_commit(handle);
    nvs_close(handle);
}

void ParamStore::saveProjectField(
    uint8_t voiceIndex,
    menu::Page page,
    uint8_t field,
    int16_t value)
{
    // 0) Must have a valid “current” project
    if (currentProjectIndex >= maxProjects)
    {
        ESP_LOGW(TAG, "saveProjectField: no project selected");
        return;
    }

    // 1) Open NVS
    nvs_handle h;
    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &h);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "saveProjectField: nvs_open failed (%d)", err);
        return;
    }

    // 2) Read the existing blob
    char key[32];
    std::snprintf(key, sizeof(key), "%s%u", KEY_PROJ_DATA, currentProjectIndex);
    ESP_LOGD(TAG, "saveProjectField: dataKey=\"%s\", voice=%u page=%u field=%u value=%d",
             key, (unsigned)voiceIndex, (unsigned)page, (unsigned)field, (int)value);

    size_t blobLen = 0;
    err = nvs_get_blob(h, key, nullptr, &blobLen);
    if (err != ESP_OK || blobLen == 0)
    {
        ESP_LOGE(TAG, "  nvs_get_blob probe failed (%d) or zero length", err);
        nvs_close(h);
        return;
    }

    // 3) Load blob into a vector of int16_t
    size_t entryCount = blobLen / sizeof(int16_t);
    std::vector<int16_t> blob(entryCount);
    err = nvs_get_blob(h, key, blob.data(), &blobLen);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "  nvs_get_blob read failed (%d)", err);
        nvs_close(h);
        return;
    }

    // 4) Compute the offset of this one field
    constexpr size_t P = static_cast<size_t>(menu::Page::_Count);
    constexpr size_t F = MAX_FIELDS;
    size_t fieldsPerVoice = P * F;

    if (voiceIndex >= entryCount / fieldsPerVoice)
    {
        ESP_LOGE(TAG, "  voiceIndex %u out of range (only %u voices worth of data)", voiceIndex, (unsigned)(entryCount / fieldsPerVoice));
        nvs_close(h);
        return;
    }

    size_t offs = voiceIndex * fieldsPerVoice + static_cast<size_t>(page) * F + static_cast<size_t>(field);

    if (offs >= entryCount)
    {
        ESP_LOGE(TAG, "  computed offset %zu out of blob range (%zu)", offs, entryCount);
        nvs_close(h);
        return;
    }

    // 5) Patch the value
    blob[offs] = value;

    // 6) Write it all back
    err = nvs_set_blob(h, key, blob.data(), blob.size() * sizeof(blob[0]));
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "  nvs_set_blob(%s) failed (%d)", key, err);
    }
    else
    {
        nvs_commit(h);
    }

    nvs_close(h);
}

void ParamStore::saveVoice(const VoiceStoreEntry &entry)
{
    // 1) Open NVS for read/write
    nvs_handle handle;
    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &handle);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "saveVoice: nvs_open RW failed (%d)", err);
        return;
    }

    uint8_t slot = entry.index;
    if (slot >= maxVoices)
    {
        ESP_LOGW(TAG, "saveVoice: slot %u out of range", slot);
        nvs_close(handle);
        return;
    }

    char key[32];
    esp_err_t rc;

    // --- save the name ---
    std::snprintf(key, sizeof(key), "%s%u", KEY_VOICE_NAME, slot);
    ESP_LOGD(TAG, "saveVoice: nameKey='%s', name='%s'",
             key,
             entry.name ? entry.name->c_str() : "<empty>");
    rc = nvs_set_str(handle, key,
                     entry.name ? entry.name->c_str() : "");
    if (rc != ESP_OK)
    {
        ESP_LOGE(TAG, "  nvs_set_str(%s) failed (%d)", key, rc);
    }

    // --- save the params blob ---
    std::snprintf(key, sizeof(key), "%s%u", KEY_VOICE_DATA, slot);
    size_t blobSize = entry.params.size() * sizeof(entry.params[0]);
    ESP_LOGD(TAG, "saveVoice: dataKey='%s', blobSize=%zu bytes (%zu entries)", key, blobSize, entry.params.size());
    rc = nvs_set_blob(handle,
                      key,
                      entry.params.data(),
                      blobSize);
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

VoiceStoreEntry ParamStore::loadVoice(uint8_t index)
{
    // 1) Prepare an empty entry
    VoiceStoreEntry entry{
        .index = index,
        .name = std::nullopt,
        .params = {}};

    if (index >= maxVoices)
    {
        ESP_LOGW(TAG, "loadVoice: slot %u out of range", index);
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
        std::snprintf(nameKey, sizeof(nameKey), "%s%u", KEY_VOICE_NAME, index);
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
        std::snprintf(dataKey, sizeof(dataKey), "%s%u", KEY_VOICE_DATA, index);

        // Probe the blob length
        size_t blobLen = 0;
        err = nvs_get_blob(handle, dataKey, nullptr, &blobLen);
        if (err == ESP_OK && blobLen > 0)
        {
            size_t count = blobLen / sizeof(int16_t);
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

// List project names along with a "loaded" flag and sensible defaults for each slot
std::vector<NameEntry> ParamStore::listProjectNames() const
{
    std::vector<NameEntry> entries(maxProjects);
    // Initialize with default names and loaded=false
    for (uint8_t i = 0; i < maxProjects; ++i)
    {
        entries[i].name = makeDisplayName(i, std::nullopt);
        entries[i].loaded = false;
    }

    nvs_handle handle;
    if (nvs_open(NVS_NAMESPACE, NVS_READONLY, &handle) != ESP_OK)
    {
        return entries;
    }

    int32_t storedCount = 0;
    if (nvs_get_i32(handle, KEY_PROJ_COUNT, &storedCount) != ESP_OK || storedCount <= 0)
    {
        nvs_close(handle);
        return entries;
    }

    // For each saved slot, overwrite name and mark loaded
    for (uint8_t i = 0; i < entries.size() && i < static_cast<uint8_t>(storedCount); ++i)
    {
        char key[32];
        size_t len = 0;
        std::snprintf(key, sizeof(key), "%s%u", KEY_PROJ_NAME, i);
        if (nvs_get_str(handle, key, nullptr, &len) == ESP_OK)
        {
            std::string name(len, '\0');
            nvs_get_str(handle, key, &name[0], &len);
            if (!name.empty())
            {
                entries[i].name = name;
            }
            entries[i].loaded = true;
        }
    }

    nvs_close(handle);
    return entries;
}

std::vector<NameEntry> ParamStore::listVoiceNames() const
{
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
