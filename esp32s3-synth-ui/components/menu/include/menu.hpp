#pragma once

#include <cstdint>
#include <functional>
#include <vector>
#include <array>
#include <string>

#include "menu_struct.hpp"
#include "encoder_range.hpp"
#include "param_cache.hpp"
#include "param_store.hpp"
#define AUTOSAVE_INTERVAL_MS (2 * 60 * 1000) // 2 minutes

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
        void init(DisplayCallback displayCallback, UpdateCallback updateCallback);
        void enterMenuPage();
        void exitPage();
        void closePopup();
        void enterPopup();
        void rotateKnob(uint8_t knob, int16_t value);
        void saveProject(int16_t slotIndex, const std::string &name);

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
        void loadVoice(int16_t slotIndex);
        void loadProject(int16_t slotIndex);

        void saveVoice(int16_t slotIndex, const std::string &name);
        void updatePageFromCache();
        void initAutosaveTask();

        uint8_t voiceCount;
        ParamCache cache;

        DisplayCallback displayCallback;
    };

} // namespace menu
