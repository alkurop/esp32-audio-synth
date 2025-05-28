#pragma once

#include <cstdint>
#include <functional>
#include <vector>
#include <array>
#include <variant>
#include "menu_struct.hpp"
#include "encoder_range.hpp"
#include "param_cache.hpp"
#include "param_store.hpp"

namespace menu
{

    // Maximum number of fields any page can have (from menu_struct)
    static constexpr uint8_t MaxFieldsPerPage = 4;
    // Total number of pages
    static constexpr uint8_t PageCount = static_cast<uint8_t>(Page::_Count);

    struct DisplayEvent
    {
        MenuState state;
    };
    struct SaveVoiceEvent
    {
        uint8_t slotIndex;
        std::string name;
    };
    struct SaveProjectEvent
    {
        uint8_t slotIndex;
        std::string name;
    };

    struct LoadVoiceEvent
    {
        uint8_t slotIndex;
    };
    struct LoadProjectEvent
    {
        uint8_t slotIndex;
    };
    using MenuEvent = std::variant<DisplayEvent, SaveVoiceEvent, SaveProjectEvent, LoadVoiceEvent, LoadProjectEvent>;

    using EventCallback = std::function<void(const MenuEvent &)>;

    template <class... Ts>
    struct overloaded : Ts...
    {
        using Ts::operator()...;
    };
    template <class... Ts>
    overloaded(Ts...) -> overloaded<Ts...>;
    /**
     * Menu controller: handles navigation and integrates
     * with ParameterStore for persistent values and presets.
     * Each rotary corresponds to one field; no explicit 'field' index is stored.
     */
    class Menu
    {
    public:
        using EventCallback = std::function<void(const MenuEvent &)>;
        explicit Menu(uint8_t voiceCount);
        void init(EventCallback eventCallback);
        void enterMenuPage();
        void exitMenuPage();
        void closePopup();
        void enterPopup();
        void rotateKnob(uint8_t knob, int8_t value);

    private:
        /// Notify the display callback of the current cached state.
        void notify();
        std::array<EncoderRange, 4> calcEncoderRanges();
        void changeValueMenuList(uint8_t knob, int8_t value);
        void changeValuePopup(uint8_t knob, int8_t value);
        void changeValuePage(uint8_t knob, int8_t value);
        bool updatePopupStateForward();
        bool updatePopupStateBack();

        uint8_t voiceCount;
        ParamCache cache;
        ParamStore paramStore;

        uint8_t voice = 0; // zero-based index internally
        MenuState state;

        EventCallback eventCb;
    };

} // namespace menu
