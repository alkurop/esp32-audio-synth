// Menu.cpp

#include <algorithm>
#include <cstdint>
#include <utility>
#include <esp_log.h>
#include <cstring>

#include "menu.hpp"
#include "menu_struct.hpp"

static const char *TAG = "Menu";
using namespace menu;

// … all your other Menu method definitions …

// Called when the user confirms “Load Voice”
void Menu::loadVoice(uint8_t slotIndex)
{
    // TODO: actually load voice number `slotIndex` from storage
    ESP_LOGI(TAG, "loadVoice(%d) called", slotIndex);
}

// Called when the user confirms “Save Voice”
void Menu::saveVoice(uint8_t slotIndex, const std::string &name)
{
    // TODO: actually save `name` into voice slot `slotIndex` in storage
    ESP_LOGI(TAG, "saveVoice(slot=%d, name=%s)", slotIndex, name.c_str());
}

// Called when the user confirms “Load Project”
void Menu::loadProject(uint8_t slotIndex)
{
    // TODO: actually load project number `slotIndex` from storage
    ESP_LOGI(TAG, "loadProject(%d) called", slotIndex);
}

// Called when the user confirms “Save Project”
void Menu::saveProject(uint8_t slotIndex, const std::string &name)
{
    // TODO: actually save `name` into project slot `slotIndex` in storage
    ESP_LOGI(TAG, "saveProject(slot=%d, name=%s)", slotIndex, name.c_str());

} // namespace menu
