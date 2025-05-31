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

void Menu::loadVoice(int16_t slotIndex)
{
    // 1) Pull back the stored entry
    const auto entry = paramStore.loadVoice(slotIndex);
    ESP_LOGI(TAG, "loadVoice(%d): name=%s, params.size=%d", slotIndex, entry.name ? entry.name->c_str() : "<none>", entry.voiceParams.size());
    if (entry.voiceParams.empty())
    {
        // nothing to restore
        notify();
        return;
    }

    // 2) Compute page & field counts
    size_t totalEntries = entry.voiceParams.size();
    size_t pageCount = totalEntries / MAX_FIELDS;
    const size_t maxPages = static_cast<size_t>(menu::Page::_Count);
    pageCount = std::min(pageCount, maxPages);

    // 3) Walk each page
    for (size_t p = 0; p < pageCount; ++p)
    {
        const auto &pi = menu::menuPages[p];
        size_t fields = pi.fieldCount;

        ESP_LOGI(TAG, "  Page %2d (%s): %d fields", p, pi.title, fields);
        // 4) Walk each field
        for (size_t f = 0; f < fields; ++f)
        {
            size_t idx = p * MAX_FIELDS + f;
            int16_t v = entry.voiceParams[idx];

            // a) Log it

            // b) Write into your in-RAM cache
            cache.set(state.voice, static_cast<menu::Page>(p), static_cast<uint8_t>(f), v);

            // c) If this is channel/volume, also update your MenuState
            if (p == static_cast<size_t>(menu::Page::Channel))
            {
                if (f == static_cast<size_t>(menu::ChannelField::Chan))
                    state.channel = static_cast<uint8_t>(v);
                else if (f == static_cast<size_t>(menu::ChannelField::Vol))
                    state.volume = static_cast<uint8_t>(v);
            }
        }
    }

    ESP_LOGI(TAG, "loadVoice(%d): done", slotIndex);
    state.shouldAutoSave = true;
    notify();
}

// Called when the user confirms “Save Voice”
void Menu::saveVoice(int16_t slotIndex, const std::string &name)
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

// Called when the user confirms “Load Project”
void Menu::loadProject(int16_t slotIndex)
{
    const auto projectEntry = paramStore.loadProject(slotIndex);
    if (projectEntry.voices.empty())
    {
        // nothing to restore
        notify();
        return;
    }
    // const auto &ve : projectEntry.voices
    for (size_t v = 0; v < projectEntry.voices.size(); ++v)
    {
        const auto &ve = projectEntry.voices[v];
        const auto &flat = ve.voiceParams;
        size_t pageCount = flat.size() / MAX_FIELDS;

        for (size_t p = 0; p < pageCount; ++p)
        {
            // grab the real number of fields on this page
            const auto &pi = menu::menuPages[p];
            size_t fields = pi.fieldCount;

            for (size_t f = 0; f < fields; ++f)
            {
                size_t idx = p * MAX_FIELDS + f;
                if (idx >= flat.size())
                    break;
                int16_t value = flat[idx];
                cache.set(
                    ve.index,
                    static_cast<menu::Page>(p),
                    static_cast<uint8_t>(f),
                    value);
                if (v == state.voice)
                {
                    // c) If this is channel/volume, also update your MenuState
                    if (p == static_cast<size_t>(menu::Page::Channel))
                    {
                        if (f == static_cast<size_t>(menu::ChannelField::Chan))
                            state.channel = static_cast<uint8_t>(v);
                        else if (f == static_cast<size_t>(menu::ChannelField::Vol))
                            state.volume = static_cast<uint8_t>(v);
                    }
                }
            }
        }
    }

    // Restore global parameters
    const auto &flatGlobal = projectEntry.globalParams;
    size_t globalPageCount = flatGlobal.size() / MAX_FIELDS;

    for (size_t p = 0; p < globalPageCount; ++p)
    {
        const auto &pi = menu::menuPages[p];
        size_t fields = pi.fieldCount;

        for (size_t f = 0; f < fields; ++f)
        {
            size_t idx = p * MAX_FIELDS + f;
            if (idx >= flatGlobal.size())
                break;
            int16_t value = flatGlobal[idx];

            // Set global field in cache
            cache.set(
                -1,
                static_cast<menu::Page>(p),
                static_cast<uint8_t>(f),
                value);
        }
    }
    notify();
    if (slotIndex != AUTOSAVE_SLOT)
    {
        state.shouldAutoSave = true;
    }
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
    paramStore.saveProject(entry, true);
}
