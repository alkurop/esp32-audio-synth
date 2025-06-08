#include "set_page.hpp"
using namespace settings;
using namespace protocol;

void settings::setGlobalPage(SoundModule &soundModule, uint8_t field, int16_t value)
{
    auto fieldType = static_cast<GlobalField>(field);
    switch (fieldType)
    {
    case GlobalField::SyncMode:
    {
        auto isSynced = static_cast<bool>(value);
        soundModule.getState().usesSettingsBmp = !isSynced;
        soundModule.updateBpmSetting();
    }
    break;
    case GlobalField::ManualBPM:{
        auto bpm = static_cast<uint16_t>(value);
        soundModule.getState().settingsBpm = bpm;
        soundModule.updateBpmSetting();
        break;
    }
    default :
        break;
    }
};
