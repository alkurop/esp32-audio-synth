#pragma once
#include <cstdint>
#include <cstddef>
#include <limits>

namespace menu
{

    enum class Workflow : uint8_t
    {
        LoadVoice = 0,
        SaveVoice,
        LoadProject,
        SaveProject,
        Count
    };

    static constexpr const char *popupMenuTitles[static_cast<size_t>(Workflow::Count)] = {
        "Load Voice",
        "Save Voice",
        "Load Project",
        "Save Project"};

    enum class PopupMode : uint8_t
    {
        LoadVoiceList,
        LoadVoiceConfirm,
        SaveVoiceList,
        SaveVoiceRename,
        SaveVoiceConfirm,
        LoadProjectList,
        LoadProjectConfirm,
        SaveProjectList,
        SaveProjectRename,
        SaveProjectConfirm,
        Count
    };

    struct PopupEntry
    {
        PopupMode mode;
        const char *title;
    };

    struct PopupWorkflow
    {
        const PopupEntry *steps;
        uint8_t stepCount;
    };

    static constexpr PopupEntry loadVoiceSteps[] = {
        {PopupMode::LoadVoiceList, "Select Voice"},
        {PopupMode::LoadVoiceConfirm, "Confirm Load"}};
    static constexpr PopupEntry saveVoiceSteps[] = {
        {PopupMode::SaveVoiceList, "Select Slot"},
        {PopupMode::SaveVoiceRename, "Rename Voice"},
        {PopupMode::SaveVoiceConfirm, "Confirm Save"}};
    static constexpr PopupEntry loadProjectSteps[] = {
        {PopupMode::LoadProjectList, "Select Project"},
        {PopupMode::LoadProjectConfirm, "Confirm Load"}};
    static constexpr PopupEntry saveProjectSteps[] = {
        {PopupMode::SaveProjectList, "Select Slot"},
        {PopupMode::SaveProjectRename, "Rename Project"},
        {PopupMode::SaveProjectConfirm, "Confirm Save"}};

    static constexpr PopupWorkflow popupWorkflows[static_cast<size_t>(Workflow::Count)] = {
        /* Workflow::LoadVoice */ {loadVoiceSteps, sizeof(loadVoiceSteps) / sizeof(PopupEntry)},
        /* Workflow::SaveVoice */ {saveVoiceSteps, sizeof(saveVoiceSteps) / sizeof(PopupEntry)},
        /* Workflow::LoadProject */ {loadProjectSteps, sizeof(loadProjectSteps) / sizeof(PopupEntry)},
        /* Workflow::SaveProject */ {saveProjectSteps, sizeof(saveProjectSteps) / sizeof(PopupEntry)}};

    struct PopupState
    {
        Workflow workflowIndex = Workflow::Count; ///< invalid until set
        uint8_t stepIndex = 0;                    ///< 0..stepCount-1
        int8_t slotIndex = -1;                    ///< voice/project slot cursor
        char editName[5] = {'\0'};                ///< 4-char rename buffer
        bool nameEditing = false;                 ///< are we editing text?
    };

    static constexpr char nameAlphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    static constexpr std::size_t nameAlphabetSize = sizeof(nameAlphabet) - 1;

    // 8) Helpers to advance/retreat
    inline bool advancePopup(PopupState &p)
    {
        auto w = static_cast<size_t>(p.workflowIndex);
        if (w >= static_cast<size_t>(Workflow::Count))
            return false;
        auto &wf = popupWorkflows[w];
        if (p.stepIndex + 1 < wf.stepCount)
        {
            ++p.stepIndex;
            return true;
        }
        return false;
    }

    inline bool retreatPopup(PopupState &p)
    {
        if (p.stepIndex > 0)
        {
            --p.stepIndex;
            return true;
        }
        return false;
    }

} // namespace menu
