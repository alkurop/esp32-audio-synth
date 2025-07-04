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
    auto entry = paramStore.loadVoice(slotIndex);
    ESP_LOGI(TAG, "loadVoice(%d): name=%s, params.size=%d",
             slotIndex,
             entry.name ? entry.name->c_str() : "<none>",
             static_cast<int>(entry.voiceParams.size()));

    FieldUpdateList updates = entry.voiceParams.empty()
                                  ? presets.loadDefaultVoice(slotIndex)
                                  : mapVoiceStoreEntryToUpdates(entry);

    if (auto channelPage = parseChannelPage(updates))
    {
        state.channel = channelPage->channel;
        state.volume = channelPage->volume;
        ESP_LOGI(TAG, "Loaded voice. Setting volume and channel in menu topbar");
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
    const ProjectStoreEntry projectEntry = paramStore.loadProject(slotIndex);
    auto updates = projectEntry.voices.empty() ? presets.loadDefaultProject() : mapProjectEntryToUpdates(projectEntry);
    // todo Update channel/volume state
    if (auto channelPage = parseChannelPage(updates))
    {
        ESP_LOGI(TAG, "Loaded project. Setting volume and channel in menu topbar");

        state.channel = channelPage->channel;
        state.volume = channelPage->volume;
    }
    // Apply all updates at once
    cache.set(updates);

    if (slotIndex != AUTOSAVE_SLOT)
    {
        state.shouldAutoSave = true;
        notify();
    }
    else
    {
        updateAfterAutoLoad();
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
    paramStore.saveProject(entry);
    if (slotIndex != AUTOSAVE_SLOT)
    {
        state.shouldAutoSave = true;
    }
}
