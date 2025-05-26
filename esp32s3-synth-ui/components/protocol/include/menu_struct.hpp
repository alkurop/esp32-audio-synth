#pragma once
#include <cstdint>
#include <array>

namespace menu
{

    enum class AppMode : uint8_t
    {
        MenuList, // page/voice selector
        Page,     // parameter editing
        Popup     // load/save dialog
    };

    //--- Popup/dialog modes for load/save workflows ---
    enum class PopupMode : uint8_t
    {
        LoadVoiceList,      // select voice to load
        LoadVoiceConfirm,   // confirm voice loaded
        SaveVoiceList,      // select voice slot to save
        SaveVoiceRename,    // rename voice popup
        SaveVoiceConfirm,   // confirm voice saved
        LoadProjectList,    // select project to load
        LoadProjectConfirm, // confirm project loaded
        SaveProjectList,    // select project slot to save
        SaveProjectRename,  // rename project popup
        SaveProjectConfirm,
        None, // regular page/knob navigation
              // confirm project saved,
    };

    // right after your PopupMode enum

    // human-readable labels for each step in the workflows:
    static constexpr const char *loadVoiceSteps[] = {"Voice List", "Confirm Load"};
    static constexpr const char *saveVoiceSteps[] = {"Voice List", "Rename Voice", "Confirm Save"};
    static constexpr const char *loadProjectSteps[] = {"Project List", "Confirm Load"};
    static constexpr const char *saveProjectSteps[] = {"Project List", "Rename Project", "Confirm Save"};

    // one struct per workflow:
    struct PopupWorkflow
    {
        const char *title;        // what appears in the main menu
        const char *const *steps; // labels for each step of that workflow
        uint8_t count;            // how many steps
        PopupMode baseMode;       // the first PopupMode for this workflow
    };

    // a constexpr array of all four:
    static constexpr PopupWorkflow popupWorkflows[] = {
        {"Load Voice", loadVoiceSteps, 2, PopupMode::LoadVoiceList},
        {"Save Voice", saveVoiceSteps, 3, PopupMode::SaveVoiceList},
        {"Load Project", loadProjectSteps, 2, PopupMode::LoadProjectList},
        {"Save Project", saveProjectSteps, 3, PopupMode::SaveProjectList},
    };

    // pages
    enum class Page : uint8_t
    {
        Channel,
        Oscillator,
        Filter,
        Envelope,
        Tuning,
        FilterLFO,
        DetuneLFO,
        BPM, ///< Global BPM settings page
        _Count
    };

    // fields per page
    enum class ChannelField : uint8_t
    {
        Chan,
        Vol,
        _Count
    };
    enum class OscillatorField : uint8_t
    {
        Shape,
        Morph,
        PWM,
        Sync,
        _Count
    };

    enum class FilterField : uint8_t
    {
        Type,
        Cut,
        Res,
        _Count
    };
    enum class EnvField : uint8_t
    {
        A,
        D,
        S,
        R,
        _Count
    };
    enum class TuningField : uint8_t
    {
        Octave,
        Semitone,
        FineTune,
        _Count
    };
    enum class LFOField : uint8_t
    {
        RateMode,
        Rate,
        Subdiv,
        Depth,
        _Count
    };
    enum class BPMField : uint8_t
    {
        SyncMode,
        ManualBPM,
        _Count
    };

    // type of control
    enum class FieldType
    {
        Range,
        Options
    };

    struct FieldInfo
    {
        const char *label;
        FieldType type;
        int min, max;
        const char *const *opts;
        uint8_t optCount;
    };

    // option lists
    static constexpr const char *yesNo[] = {"Off", "On"};

    // channel fields
    static constexpr FieldInfo channelInfo[] = {
        {"Chan", FieldType::Range, 0, 15, nullptr, 0},
        {"Vol", FieldType::Range, 0, 15, nullptr, 0},
    };

    // oscillator fields
    static constexpr const char *oscShapes[] = {"Sine", "Saw", "Square", "Tri"};
    static constexpr FieldInfo oscInfo[] = {
        {"Shape", FieldType::Options, 0, 0, oscShapes, 4},
        {"Morph", FieldType::Range, 0, 31, nullptr, 0},
        {"PWM", FieldType::Range, 0, 31, nullptr, 0},
        {"Sync", FieldType::Options, 0, 0, yesNo, 2},
    };

    // filter fields
    static constexpr const char *filtTypes[] = {"LP12", "HP12", "BP12", "Notch"};
    static constexpr FieldInfo filterInfo[] = {
        {"Type", FieldType::Options, 0, 0, filtTypes, 4},
        {"Cut", FieldType::Range, 0, 31, nullptr, 0},
        {"Res", FieldType::Range, 0, 31, nullptr, 0},
    };

    // envelope fields
    static constexpr FieldInfo envInfo[] = {
        {"A", FieldType::Range, 0, 31, nullptr, 0},
        {"D", FieldType::Range, 0, 31, nullptr, 0},
        {"S", FieldType::Range, 0, 31, nullptr, 0},
        {"R", FieldType::Range, 0, 31, nullptr, 0},
    };

    // tuning fields
    static constexpr FieldInfo tuningInfo[] = {
        {"Oct", FieldType::Range, -2, 2, nullptr, 0},
        {"Semi", FieldType::Range, -12, 12, nullptr, 0},
        {"Fine", FieldType::Range, -50, 50, nullptr, 0},
    };

    // LFO fields
    static constexpr const char *subdivisions[] = {"1/1", "1/2", "1/4", "1/8", "1/16", "1/32"};
    static constexpr const char *rateModes[] = {"Free", "Sync"};
    static constexpr FieldInfo lfoInfo[] = {
        {"Mode", FieldType::Options, 0, 0, rateModes, 2},
        {"Rate", FieldType::Range, 0, 31, nullptr, 0},
        {"Subd", FieldType::Options, 0, 0, subdivisions, 6},
        {"Dept", FieldType::Range, 0, 31, nullptr, 0},
    };

    // BPM fields
    static constexpr FieldInfo bpmInfo[] = {
        {"Sync", FieldType::Options, 0, 0, yesNo, 2},
        {"BPM", FieldType::Range, 30, 300, nullptr, 0},
    };

    struct PageInfo
    {
        const char *title;
        const FieldInfo *fields;
        uint8_t fieldCount;
    };

    static constexpr PageInfo menuPages[] = {
        {"Vol/Channel", channelInfo, sizeof(channelInfo) / sizeof(FieldInfo)},
        {"Oscillator", oscInfo, sizeof(oscInfo) / sizeof(FieldInfo)},
        {"Filter", filterInfo, sizeof(filterInfo) / sizeof(FieldInfo)},
        {"Envelope", envInfo, sizeof(envInfo) / sizeof(FieldInfo)},
        {"Tuning", tuningInfo, sizeof(tuningInfo) / sizeof(FieldInfo)},
        {"Filter LFO", lfoInfo, sizeof(lfoInfo) / sizeof(FieldInfo)},
        {"Detune LFO", lfoInfo, sizeof(lfoInfo) / sizeof(FieldInfo)},
        {"Global BPM", bpmInfo, sizeof(bpmInfo) / sizeof(FieldInfo)},
    };

    static_assert((uint8_t)Page::_Count == (sizeof(menuPages) / sizeof(PageInfo)), "Page count mismatch");

    /** Popup/dialog overlay state */
    struct PopupState
    {
        PopupMode mode = PopupMode::None; ///< current overlay mode
        int8_t slotIndex = -1;            ///< selected slot (voice or project)
        char editName[5] = {'\0'};        ///< 4-char rename buffer
        bool nameEditing = false;         ///< in rename edit
    };
    /// Simple [min,max] for an encoder knob.
    struct EncoderRange
    {
        uint8_t min;
        uint8_t max;
    };
    /**
     * Holds the full menu state for UI callbacks.
     */
    struct MenuState
    {
        AppMode mode;          // which of the 3 modes we’re in
        uint8_t menuItemIndex; ///< Current menu page
        uint8_t voice;         ///< Current voice number (1..N)
        uint8_t channel;       ///< Current channel for that voice
        uint8_t volume;        ///< Current volume for that voice
        int8_t fieldValues[4]; ///< Current values for each knob

        PopupState popup; ///< active load/save overlay
        std::array<EncoderRange, 4> encoderRanges;
    };

    static constexpr uint8_t pageCnt = static_cast<uint8_t>(menu::Page::_Count);
    static constexpr uint8_t workflowCnt = sizeof(popupWorkflows) / sizeof(*popupWorkflows);
    static constexpr uint8_t menuItemCnt = pageCnt + workflowCnt;

    static constexpr char nameAlphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    static constexpr std::size_t nameAlphabetSize = sizeof(nameAlphabet) - 1;

    /// “Is this index a real Page?”
    static inline bool isPageItem(uint8_t itemIndex)
    {
        return itemIndex < pageCnt;
    }

    /// Map a page‐list index into your Page enum (valid only if isPageItem() is true)
    static inline Page itemToPage(uint8_t itemIndex)
    {
        return static_cast<Page>(itemIndex);
    }

    /// “Is this index one of your popup workflows?”
    static inline bool isWorkflowItem(uint8_t itemIndex)
    {
        return itemIndex >= pageCnt && itemIndex < menuItemCnt;
    }

    /// Map a workflow‐list index into your PopupWorkflow (valid only if isWorkflowItem() is true)
    static inline const PopupWorkflow &itemToWorkflow(uint8_t itemIndex)
    {
        return popupWorkflows[itemIndex - pageCnt];
    }
} // namespace menu
