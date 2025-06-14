// Menu.cpp

#include <algorithm>
#include <cstdint>
#include <utility>
#include <esp_log.h>
#include <cstring>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "menu.hpp"
#include "menu_struct.hpp"
#include "load_save_mapper.hpp"

static const char *TAG = "Menu";
using namespace menu;
using namespace protocol;

static void autoSaveTask(void *param)
{
    Menu *menu = static_cast<Menu *>(param);

    while (true)
    {
        vTaskDelay(pdMS_TO_TICKS(AUTOSAVE_INTERVAL_MS));

        if (menu->state.shouldAutoSave)
        {
            ESP_LOGI("Autosave", "Autosaving project to slot %d...", AUTOSAVE_SLOT);
            menu->state.shouldAutoSave = false;
            menu->saveProject(AUTOSAVE_SLOT, "autosave");
            ESP_LOGI("Autosave", "Autosave completed.");
        }
    }
}

void Menu::initAutosaveTask()
{
    xTaskCreate(
        autoSaveTask,   // Task function
        "AutoSaveTask", // Name (for debugging)
        4096,           // Stack size (in words, 4KB here)
        this,           // Parameter to pass to the task
        1,              // Priority (low priority is good)
        nullptr         // Task handle (optional, pass &handle if needed)
    );
}

void Menu::loadVoice(uint8_t slotIndex)
{
    // 1) Pull back the stored entry
    const auto entry = paramStore.loadVoice(slotIndex);
    ESP_LOGI(TAG, "loadVoice(%d): name=%s, params.size=%d",
             slotIndex,
             entry.name ? entry.name->c_str() : "<none>",
             static_cast<int>(entry.voiceParams.size()));

    if (entry.voiceParams.empty())
    {
        notify();
        return;
    }

    // 2) Compute page & field counts
    size_t totalEntries = entry.voiceParams.size();
    size_t pageCount = totalEntries / MAX_FIELDS;
    const size_t maxPages = static_cast<size_t>(Page::_Count);
    pageCount = std::min(pageCount, maxPages);

    FieldUpdateList updates;

    // 3) Walk each page
    for (size_t p = 0; p < pageCount; ++p)
    {
        const auto &pi = menuPages[p];
        size_t fields = pi.fieldCount;

        ESP_LOGI(TAG, "  Page %2d (%s): %d fields", static_cast<int>(p), pi.title, static_cast<int>(fields));

        // 4) Walk each field
        for (size_t f = 0; f < fields; ++f)
        {
            size_t idx = p * MAX_FIELDS + f;
            if (idx >= entry.voiceParams.size())
                continue;

            int16_t v = entry.voiceParams[idx];

            updates.push_back(FieldUpdate{
                .voiceIndex = static_cast<uint8_t>(state.voiceIndex),
                .pageByte = static_cast<uint8_t>(p),
                .field = static_cast<uint8_t>(f),
                .value = v});

            // c) Update channel/volume state
            if (p == static_cast<size_t>(Page::Channel))
            {
                if (f == static_cast<size_t>(ChannelField::Chan))
                    state.channel = static_cast<uint8_t>(v);
                else if (f == static_cast<size_t>(ChannelField::Vol))
                    state.volume = static_cast<uint8_t>(v);
            }
        }
    }

    // 5) Apply updates to cache
    cache.set(updates);

    ESP_LOGI(TAG, "loadVoice(%d): done", slotIndex);
    state.shouldAutoSave = true;
    notify();
}

// Called when the user confirms “Save Voice”
void Menu::saveVoice(uint8_t slotIndex, const std::string &name)
{
    const auto &voiceCache = cache.getVoiceData();
    const auto &vc = voiceCache.at(slotIndex); // PageCache vector for that voice
    auto flatParams = flattenVoiceParams(vc);
    VoiceStoreEntry entry = {
        .index = slotIndex,
        .name = name,
        .voiceParams = flatParams};

    paramStore.saveVoice(entry);
}

void Menu::loadProject(int16_t slotIndex)
{
    const auto projectEntry = paramStore.loadProject(slotIndex);
    if (projectEntry.voices.empty())
    {
        notify();
        return;
    }

    FieldUpdateList updates;

    // Voice parameter updates
    for (size_t voiceIndex = 0; voiceIndex < projectEntry.voices.size(); ++voiceIndex)
    {
        const auto &ve = projectEntry.voices[voiceIndex];
        const auto &flat = ve.voiceParams;
        size_t pageCount = flat.size() / MAX_FIELDS;

        for (size_t pageIndex = 0; pageIndex < pageCount; ++pageIndex)
        {
            const auto &pageItem = menuPages[pageIndex];
            size_t fields = pageItem.fieldCount;

            for (size_t field = 0; field < fields; ++field)
            {
                size_t idx = pageIndex * MAX_FIELDS + field;
                if (idx >= flat.size())
                    break;

                int16_t value = flat[idx];
                updates.push_back(FieldUpdate{ve.index, static_cast<uint8_t>(pageIndex), static_cast<uint8_t>(field), value});

                if (voiceIndex == state.voiceIndex)
                {
                    if (pageIndex == static_cast<size_t>(Page::Channel))
                    {
                        if (field == static_cast<size_t>(ChannelField::Chan))
                            state.channel = static_cast<uint8_t>(value);
                        else if (field == static_cast<size_t>(ChannelField::Vol))
                            state.volume = static_cast<uint8_t>(value);
                    }
                }
            }
        }
    }

    // Global parameter updates
    const auto &flatGlobal = projectEntry.globalParams;
    if (!flatGlobal.empty())
    {
        for (size_t p = 0; p < GLOBAL_PAGE_COUNT; ++p)
        {
            size_t pageIndex = VOICE_PAGE_COUNT + p;
            if (pageIndex >= PAGE_COUNT)
                break;

            const auto &pi = menuPages[pageIndex];
            size_t fields = std::min<size_t>(pi.fieldCount, MAX_FIELDS);

            for (size_t f = 0; f < fields; ++f)
            {
                size_t idx = p * MAX_FIELDS + f;
                if (idx >= flatGlobal.size())
                    break;

                int16_t value = flatGlobal[idx];
                updates.push_back(FieldUpdate{static_cast<uint8_t>(-1), static_cast<uint8_t>(pageIndex), static_cast<uint8_t>(f), value});
            }
        }
    }

    // Apply all updates at once
    cache.set(updates);

    if (slotIndex != AUTOSAVE_SLOT)
    {
        state.shouldAutoSave = true;
    }
    else
    {
        state.mode = AppMode::MenuList;
    }
    notify();
}

void Menu::saveProject(int16_t slotIndex, const std::string &name)
{
    ProjectStoreEntry entry{
        .index = slotIndex,
        .name = name,
        .voices = {},
        .globalParams = flattenGlobalParams(cache.getGlobalCache())};

    const auto &voiceData = cache.getVoiceData();
    entry.voices.reserve(voiceData.size());
    for (size_t i = 0; i < voiceData.size(); ++i)
    {
        const VoiceCache &vc = voiceData[i];
        auto flatParams = flattenVoiceParams(vc);

        VoiceStoreEntry ve{
            .index = static_cast<uint8_t>(i),
            .name = std::nullopt,
            .voiceParams = std::move(flatParams)};
        entry.voices.push_back(std::move(ve));
    }

    // 5) Save it
    paramStore.saveProject(entry);
    state.shouldAutoSave = true;
}
