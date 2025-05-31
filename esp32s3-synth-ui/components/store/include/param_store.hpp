#pragma once

#include <cstdint>
#include <vector>
#include <cinttypes>
#include "param_struct.hpp" // ProjectStoreEntry, VoiceStoreEntry
#include "menu_struct.hpp"
#include <mutex>
using namespace protocol;
namespace store
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
        explicit ParamStore(uint8_t maxProjects = 32, uint8_t maxVoices = 32);

        void saveProject(const ProjectStoreEntry &entry);

        ProjectStoreEntry loadProject(int16_t index);

        std::vector<NameEntry> listProjectNames();

        void saveVoice(const VoiceStoreEntry &entry);

        VoiceStoreEntry loadVoice(uint8_t index);

        std::vector<NameEntry> listVoiceNames();

    private:
        uint8_t maxProjects;
        uint8_t maxVoices;
        std::mutex nvsMutex;
    };

} // namespace menu
