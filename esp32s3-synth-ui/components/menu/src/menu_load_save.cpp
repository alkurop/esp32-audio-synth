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
    const auto voiceEntry = paramStore.loadVoice(slotIndex);
    const auto &flat = voiceEntry.params;
    if (flat.empty())
        return;
    size_t pageCount = flat.size() / MAX_FIELDS;
    for (size_t p = 0; p < pageCount; ++p)
    {
        for (size_t f = 0; f < MAX_FIELDS; ++f)
        {
            size_t idx = p * MAX_FIELDS + f;
            if (idx >= flat.size())
                break;
            int16_t value = flat[idx];
            cache.set(slotIndex, static_cast<menu::Page>(p), static_cast<uint8_t>(f), value);
        }
    }
    auto voiceData = unflattenVoiceParams(voiceEntry.params);
    auto pageIndex = static_cast<size_t>(menu::Page::Channel);
    auto chanFieldIndex = static_cast<size_t>(menu::ChannelField::Chan);
    auto volFieldIndex = static_cast<size_t>(menu::ChannelField::Vol);
    int16_t channel = voiceData[pageIndex][chanFieldIndex];
    int16_t volume = voiceData[pageIndex][volFieldIndex];
    state.channel = channel;
    state.volume = volume;

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
