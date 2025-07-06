#pragma once

#include <cstdint>
#include <functional>
#include <vector>
#include <array>
#include <string>

#include "menu_struct.hpp"
#include "menu_state.hpp"
#include "encoder_range.hpp"
#include "cache.hpp"
#include "param_store.hpp"
#include "popup_struct.hpp"
#include "presets.hpp"


using namespace protocol;
using namespace store;

namespace menu
{

    using DisplayCallback = std::function<void(const MenuState &state)>;

    /**
     * Menu controller: handles navigation and integrates
     * with ParameterStore for persistent values and presets.
     */
    class Menu
    {
    public:
        explicit Menu(uint8_t voiceCount);
        void init(DisplayCallback displayCallback, FieldUpdateCallback updateCallback);
        void enterMenuPage();
        void exitPage();
        void closePopup();
        void enterPopup();
        void rotateKnob(uint8_t knob, int16_t value);
        void saveProject(int16_t slotIndex, const std::string &name);
        void updateAfterAutoLoad();
        void voiceUp();
        void voiceDown();

        // for autosave task
        ParamStore paramStore;
        MenuState state;

    private:
        /// Notify the display callback of the current cached state.
        void notify();

        /// Calculate the encoder ranges based on current mode & state
        std::array<EncoderRange, 4> calcEncoderRanges();

        /// Handlers for knob changes in each mode
        void changeValueMenuList(uint8_t knob, int16_t value);
        void changeValuePage(uint8_t knob, int16_t value);
        void changeValuePopup(uint8_t knob, int16_t value);

        /// Advance or retreat in a popup workflow
        bool updatePopupStateForward();
        bool updatePopupStateBack();

        /// Internal actions for loading/saving
        void loadVoice(uint8_t slotIndex);
        void loadProject(int16_t slotIndex);

        void saveVoice(uint8_t slotIndex, const std::string &name);
        void updatePageFromCache();
        void initAutosaveTask();
        void updateVoiceUp();

        uint8_t voiceCount;
        Cache cache;

        DisplayCallback displayCallback;
        Presets presets;
    };

} // namespace menu
