#pragma once

#include <cstdint>
#include <vector>
#include <cinttypes>
#include "param_struct.hpp" // ProjectStoreEntry, VoiceStoreEntry
#include "menu_struct.hpp"

namespace menu
{

    /**
     * Manages in-memory parameter presets and persists them to NVS.
     * Supports named project- and voice-presets, each stored in fixed slots.
     */
    class ParamStore
    {
    public:
        /**
         * @param maxProjects   Maximum number of project slots available.
         * @param maxVoices     Maximum number of voice slots available.
         */
        explicit ParamStore(uint8_t maxProjects = 32,
                            uint8_t maxVoices = 32);

        // --- Project-level operations ---

        /**
         * Save all voice parameters in the given project slot.
         */
        void saveProject(const ProjectStoreEntry &entry);

        /**
         * Load parameters from the project slot at index.
         * @return true if the slot contains a valid project.
         */
        ProjectStoreEntry loadProject(uint8_t index);

        /**
         * List all stored project names (most-recent first).
         */
        std::vector<NameEntry> listProjectNames() const;

        /**
         * Get the index of the currently loaded project, or -1.
         */
        uint8_t getCurrentProjectIndex() const;

        // --- Global field operations ---
        std::vector<int16_t> getGlobalFields() const;

        /**
         * Save a single global (project-level) parameter value.
         */
        void saveGlobalField(Page page, uint8_t field, int16_t value);

        // --- Project field operations ---

        /**
         * Save a single field for a specific voice in the current project.
         */
        void saveProjectField(uint8_t voice, Page page, uint8_t field, int16_t value);

        // --- Voice-level operations ---

        /**
         * Save a single voice preset under the given slot.
         */
        void saveVoice(const VoiceStoreEntry &entry);

        /**
         * Load a single voice preset from the given slot.
         * @return true if the slot contains a valid voice.
         */
        VoiceStoreEntry loadVoice(uint8_t index);

        /**
         * List all stored voice names (most-recent first).
         */
        std::vector<NameEntry> listVoiceNames() const;

    private:
        uint8_t maxProjects;
        uint8_t maxVoices;

        uint8_t currentProjectIndex = 0;
    };

} // namespace menu
