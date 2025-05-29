#include <esp_err.h>
#include <nvs_flash.h>
#include <algorithm>
#include <cstdio>
#include <string>
#include "param_store.hpp"

namespace menu
{
    static constexpr char NVS_NAMESPACE[] = "synth";
    static constexpr char KEY_PROJ_COUNT[] = "proj_count";
    static constexpr char KEY_PROJ_NAME[] = "proj_name_";
    static constexpr char KEY_PROJ_DATA[] = "proj_data_";
    static constexpr char KEY_GLOB_FIELD[] = "glob_field_";
    static constexpr char KEY_VOICE_COUNT[] = "voice_count";
    static constexpr char KEY_VOICE_NAME[] = "voice_name_";
    static constexpr char KEY_VOICE_DATA[] = "voice_data_";
    static constexpr char KEY_PROJ_VOICE_COUNT[] = "proj_voice_count_"; // + project slot
    static constexpr char KEY_PROJ_VOICE_NAME[] = "proj_voice_name_";   // + project slot + "_" + voice index

    static constexpr uint8_t MAX_FIELDS = 4;

    ParamStore::ParamStore(uint8_t maxProjects, uint8_t maxVoices) : maxProjects(maxProjects), maxVoices(maxVoices)
    {
        nvs_flash_init();
    }

    void ParamStore::saveProject(const ProjectStoreEntry &entry)
    {
        if (entry.index >= maxProjects)
            return;

        // 1) Flatten all voices' params into one big vector
        const size_t P = static_cast<size_t>(Page::_Count);
        const size_t F = MAX_FIELDS;
        std::vector<int16_t> blob;
        blob.reserve(entry.voices.size() * P * F);

        for (const auto &ve : entry.voices)
        {
            // ensure ve.params has exactly P*F entries
            blob.insert(blob.end(), ve.params.begin(), ve.params.end());
        }

        // 2) Open NVS
        nvs_handle h;
        if (nvs_open(NVS_NAMESPACE, NVS_READWRITE, &h) != ESP_OK)
            return;

        // 3) Save the flattened blob
        {
            std::string dataKey = std::string(KEY_PROJ_DATA) + std::to_string(entry.index);
            nvs_set_blob(h,
                         dataKey.c_str(),
                         blob.data(),
                         blob.size() * sizeof(blob[0]));
        }

        // 4) Save how many voices
        {
            std::string cntKey = std::string(KEY_PROJ_VOICE_COUNT) + std::to_string(entry.index);
            nvs_set_i32(h, cntKey.c_str(), static_cast<int32_t>(entry.voices.size()));
        }

        // 5) Save each voice's optional name
        for (size_t i = 0; i < entry.voices.size(); ++i)
        {
            const auto &ve = entry.voices[i];
            std::string nameKey = std::string(KEY_PROJ_VOICE_NAME) + std::to_string(entry.index) + "_" + std::to_string(i);
            nvs_set_str(h,
                        nameKey.c_str(),
                        ve.name ? ve.name->c_str() : "");
        }

        nvs_commit(h);
        nvs_close(h);

        currentProjectIndex = entry.index;
    }

    ProjectStoreEntry ParamStore::loadProject(uint8_t index)
    {
        ProjectStoreEntry entry;
        entry.index = index;
        if (index >= maxProjects)
            return entry;

        nvs_handle h;
        if (nvs_open(NVS_NAMESPACE, NVS_READONLY, &h) != ESP_OK)
            return entry;

        // 1) Read how many voices
        int32_t count = 0;
        {
            std::string cntKey = std::string(KEY_PROJ_VOICE_COUNT) + std::to_string(index);
            if (nvs_get_i32(h, cntKey.c_str(), &count) != ESP_OK || count <= 0)
            {
                nvs_close(h);
                return entry;
            }
        }

        const size_t P = static_cast<size_t>(Page::_Count);
        const size_t F = MAX_FIELDS;
        const size_t totalFields = static_cast<size_t>(count) * P * F;
        std::vector<int16_t> blob(totalFields);

        // 2) Read the big data blob
        {
            std::string dataKey = std::string(KEY_PROJ_DATA) + std::to_string(index);
            size_t blobBytes = totalFields * sizeof(int16_t);
            if (nvs_get_blob(h, dataKey.c_str(), blob.data(), &blobBytes) != ESP_OK)
            {
                nvs_close(h);
                return entry;
            }
        }

        // 3) Split blob into VoiceStoreEntry objects
        entry.voices.clear();
        entry.voices.reserve(count);
        for (int32_t v = 0; v < count; ++v)
        {
            VoiceStoreEntry ve;
            ve.index = static_cast<uint8_t>(v);
            ve.params.resize(P * F);
            std::copy_n(
                blob.begin() + v * P * F,
                P * F,
                ve.params.begin());

            // load optional name
            std::string nameKey = std::string(KEY_PROJ_VOICE_NAME) + std::to_string(index) + "_" + std::to_string(v);
            size_t len = 0;
            if (nvs_get_str(h, nameKey.c_str(), nullptr, &len) == ESP_OK)
            {
                std::string nm(len, '\0');
                nvs_get_str(h, nameKey.c_str(), &nm[0], &len);
                if (!nm.empty())
                    ve.name = nm;
            }

            entry.voices.push_back(std::move(ve));
        }

        nvs_close(h);
        currentProjectIndex = index;
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
        uint8_t voice,
        Page page,
        uint8_t field,
        int16_t value)
    {
        if (currentProjectIndex >= maxProjects)
            return;

        // 1. Load existing project blob
        char dataKey[32];
        std::snprintf(dataKey, sizeof(dataKey), "%s%u", KEY_PROJ_DATA, currentProjectIndex);

        nvs_handle h;
        if (nvs_open(NVS_NAMESPACE, NVS_READWRITE, &h) != ESP_OK)
            return;

        // fetch blob length
        size_t blobLen = 0;
        if (nvs_get_blob(h, dataKey, nullptr, &blobLen) != ESP_OK)
        {
            nvs_close(h);
            return;
        }

        // load blob into vector<int16_t>
        size_t count = blobLen / sizeof(int16_t);
        std::vector<int16_t> blob(count);
        nvs_get_blob(h, dataKey, blob.data(), &blobLen);

        // 2. compute offset
        const size_t P = static_cast<size_t>(Page::_Count);
        const size_t F = P * MAX_FIELDS;
        size_t v = (voice - 1);
        size_t base = v * F;
        size_t offs = base + static_cast<size_t>(page) * MAX_FIELDS + field;
        if (offs >= blob.size())
        {
            nvs_close(h);
            return;
        }

        // 3. patch the value
        blob[offs] = value;

        // 4. write it all back
        nvs_set_blob(h, dataKey, blob.data(), blob.size() * sizeof(blob[0]));
        nvs_commit(h);
        nvs_close(h);
    }

    void ParamStore::saveVoice(const VoiceStoreEntry &entry)
    {
        nvs_handle handle;
        if (nvs_open(NVS_NAMESPACE, NVS_READWRITE, &handle) != ESP_OK)
            return;

        uint8_t slot = entry.index;
        if (slot >= maxVoices)
        {
            nvs_close(handle);
            return;
        }

        // --- save the name ---
        char key[32];
        std::snprintf(key, sizeof(key), "%s%u", KEY_VOICE_NAME, slot);
        nvs_set_str(handle, key, entry.name ? entry.name->c_str() : "");

        // --- save the blob ---
        std::snprintf(key, sizeof(key), "%s%u", KEY_VOICE_DATA, slot);
        nvs_set_blob(handle, key,
                     entry.params.data(),
                     entry.params.size() * sizeof(entry.params[0]));

        // --- update voice_count if this slot is past the old count ---
        int32_t oldCount = 0;
        nvs_get_i32(handle, KEY_VOICE_COUNT, &oldCount);
        int32_t newCount = std::max<int32_t>(oldCount, slot + 1);
        nvs_set_i32(handle, KEY_VOICE_COUNT, newCount);

        nvs_commit(handle);
        nvs_close(handle);
    }

    VoiceStoreEntry ParamStore::loadVoice(uint8_t index)
    {
        VoiceStoreEntry entry{
            .index = index,
            .name = std::nullopt,
            .params = {},

        };
        if (index >= maxVoices)
            return entry;

        nvs_handle handle;
        if (nvs_open(NVS_NAMESPACE, NVS_READONLY, &handle) != ESP_OK)
            return entry;

        char nameKey[32];
        size_t len = 0;
        std::snprintf(nameKey, sizeof(nameKey), "%s%u", KEY_VOICE_NAME, index);
        if (nvs_get_str(handle, nameKey, nullptr, &len) == ESP_OK)
        {
            std::string name(len, '\0');
            nvs_get_str(handle, nameKey, &name[0], &len);
            if (!name.empty())
                entry.name = name;
        }

        char dataKey[32];
        size_t blobLen = 0;
        std::snprintf(dataKey, sizeof(dataKey), "%s%u", KEY_VOICE_DATA, index);
        if (nvs_get_blob(handle, dataKey, nullptr, &blobLen) == ESP_OK)
        {
            size_t count = blobLen / sizeof(int16_t);
            entry.params.resize(count);
            nvs_get_blob(handle, dataKey, entry.params.data(), &blobLen);
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

} // namespace menu
