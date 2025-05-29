// Menu.cpp

#include <algorithm>
#include <cstdint>
#include <utility>
#include <esp_log.h>
#include <cstring>

#include "menu.hpp"
#include "menu_struct.hpp"
#include "load_save_mapper.hpp"

static const char *TAG = "Menu";
using namespace menu;

void Menu::loadVoice(uint8_t slotIndex)
{
    // 1) Pull back the stored entry
    const auto entry = paramStore.loadVoice(slotIndex);
    ESP_LOGI(TAG, "loadVoice(%u): name=%s, params.size=%u",
             slotIndex,
             entry.name ? entry.name->c_str() : "<none>",
             (unsigned)entry.params.size());

    if (entry.params.empty())
    {
        // nothing to restore
        return;
    }

    // 2) Compute page & field counts
    size_t totalEntries = entry.params.size();
    size_t pageCount = totalEntries / MAX_FIELDS;
    const size_t maxPages = static_cast<size_t>(menu::Page::_Count);
    pageCount = std::min(pageCount, maxPages);

    // 3) Walk each page
    for (size_t p = 0; p < pageCount; ++p)
    {
        const auto &pi = menu::menuPages[p];
        size_t fields = pi.fieldCount;

        ESP_LOGI(TAG, "  Page %2u (%s): %u fields",
                 (unsigned)p, pi.title, (unsigned)fields);

        // 4) Walk each field
        for (size_t f = 0; f < fields; ++f)
        {
            size_t idx = p * MAX_FIELDS + f;
            int16_t v = entry.params[idx];

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

    // 5) (Optional) If you want to immediately reflect the loaded voice on-screen:
    //    enterMenuPage(Page::Channel);
    //    callbackUpdateKnobs();  // however your UI picks up state.channel/volume

    ESP_LOGI(TAG, "loadVoice(%u): done", slotIndex);

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
        .params = flatParams};

    paramStore.saveVoice(entry);
}

// Called when the user confirms “Load Project”
void Menu::loadProject(uint8_t slotIndex)
{
    const auto projectEntry = paramStore.loadProject(slotIndex);
    for (const auto &ve : projectEntry.voices)
    {
        const auto &flat = ve.params;
        size_t pageCount = flat.size() / MAX_FIELDS;
        for (size_t p = 0; p < pageCount; ++p)
        {
            for (size_t f = 0; f < MAX_FIELDS; ++f)
            {
                size_t idx = p * MAX_FIELDS + f;
                int16_t value = flat[idx];
                cache.set(ve.index, static_cast<menu::Page>(p), static_cast<uint8_t>(f), value);
            }
        }
    }
}

void Menu::saveProject(uint8_t slotIndex, const std::string &name)
{
    ProjectStoreEntry entry{
        .index = slotIndex,
        .name = name,
        .voices = {}};

    const auto &voiceData = cache.getVoiceData();
    entry.voices.reserve(voiceData.size());
    for (size_t i = 0; i < voiceData.size(); ++i)
    {
        const VoiceCache &vc = voiceData[i];
        auto flatParams = flattenVoiceParams(vc);

        VoiceStoreEntry ve{
            .index = static_cast<uint8_t>(i),
            .name = std::nullopt,
            .params = std::move(flatParams)};
        entry.voices.push_back(std::move(ve));
    }

    // 5) Save it
    paramStore.saveProject(entry);
}
