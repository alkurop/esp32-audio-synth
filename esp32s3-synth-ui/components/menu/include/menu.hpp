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
        void init(DisplayCallback displayCallback);
        void enterMenuPage();
        void exitMenuPage();
        void closePopup();
        void enterPopup();
        void rotateKnob(uint8_t knob, uint8_t value);

    private:
        /// Notify the display callback of the current cached state.
        void notify();

        /// Calculate the encoder ranges based on current mode & state
        std::array<EncoderRange, 4> calcEncoderRanges();

        /// Handlers for knob changes in each mode
        void changeValueMenuList(uint8_t knob, uint8_t value);
        void changeValuePage(uint8_t knob, uint8_t value);
        void changeValuePopup(uint8_t knob, uint8_t value);

        /// Advance or retreat in a popup workflow
        bool updatePopupStateForward();
        bool updatePopupStateBack();

        /// Internal actions for loading/saving
        void loadVoice(uint8_t slotIndex);
        void saveVoice(uint8_t slotIndex, const std::string &name);
        void loadProject(uint8_t slotIndex);
        void saveProject(uint8_t slotIndex, const std::string &name);
        void updateFieldValuesFromCache();
        
        uint8_t voiceCount;
        ParamCache cache;
        ParamStore paramStore;

        MenuState state;
        DisplayCallback displayCallback;
    };

} // namespace menu
