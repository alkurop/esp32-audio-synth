#pragma once
#include "protocol.hpp"

using namespace protocol;
namespace store
{
    class Presets

    {
    private:
        FieldUpdateList loadGlobalFields();

        template <size_t N>
        FieldUpdateList loadFieldDefaults(
            uint8_t voiceIndex,
            Page page,
            const FieldInfo (&fieldInfoArray)[N]);

        /* data */
    public:
        FieldUpdateList loadDefaultProject();
        FieldUpdateList loadDefaultVoice(uint8_t voiceIndex);
    };

}
