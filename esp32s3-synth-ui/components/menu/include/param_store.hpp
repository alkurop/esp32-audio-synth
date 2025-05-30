#pragma once

#include <cstdint>
#include <vector>
#include <cinttypes>
#include "param_struct.hpp" // ProjectStoreEntry, VoiceStoreEntry
#include "menu_struct.hpp"
#include <mutex>

namespace menu
{
    static constexpr int8_t AUTOSAVE_SLOT = -1;
    static constexpr char NVS_NAMESPACE[] = "synth";
    static constexpr char KEY_PROJ_NAME[] = "pn_";
    static constexpr char KEY_VOICE_NAME[] = "vn_";

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
        void saveProject(const ProjectStoreEntry &entry, bool allowAutosave = true);

        /**
         * Load parameters from the project slot at index.
         * @return true if the slot contains a valid project.
         */
        ProjectStoreEntry loadProject(int16_t index);

        /**
         * List all stored project names (most-recent first).
         */
        std::vector<NameEntry> listProjectNames();

        // --- Global field operations ---
        std::vector<int16_t> getGlobalFields();

        /**
         * Save a single global (project-level) parameter value.
         */
        void saveGlobalField(Page page, uint8_t field, int16_t value);
        // --- Voice-level operations ---

        /**
         * Save a single voice preset under the given slot.
         */
        void saveVoice(const VoiceStoreEntry &entry);

        /**
         * Load a single voice preset from the given slot.
         * @return true if the slot contains a valid voice.
         */
        VoiceStoreEntry loadVoice(int16_t index);

        /**
         * List all stored voice names (most-recent first).
         */
        std::vector<NameEntry> listVoiceNames();

    private:
        uint8_t maxProjects;
        uint8_t maxVoices;
        std::mutex nvsMutex;
    };

} // namespace menu
