#include "presets.hpp"

using namespace store;

FieldUpdateList Presets::loadDefaultProject()
{
    FieldUpdateList result;
    for (int voiceIndex = 0; voiceIndex < NUM_VOICES; voiceIndex++)
    {
        auto voice = loadDefaultVoice(voiceIndex);
        result.insert(result.end(), voice.begin(), voice.end());
    }
    auto globalFields = loadGlobalFields();
    result.insert(result.end(), globalFields.begin(), globalFields.end());
    return result;
}

FieldUpdateList Presets::loadGlobalFields()
{
    FieldUpdateList result;
    auto fieldDefaults = loadFieldDefaults(AUTOSAVE_SLOT, Page::Global, channelInfo);
    result.insert(result.end(), fieldDefaults.begin(), fieldDefaults.end());
    return result;
}

FieldUpdateList Presets::loadDefaultVoice(uint8_t voiceIndex)
{
    FieldUpdateList result;
    for (uint8_t pageIndex = 0; pageIndex < static_cast<uint8_t>(Page::_Count); ++pageIndex)
    {
        Page page = static_cast<Page>(pageIndex);

        switch (page)
        {
        case Page::Channel:
        {
            auto fieldDefaults = loadFieldDefaults(voiceIndex, page, channelInfo);
            if (voiceIndex == 0)
            {
                fieldDefaults[static_cast<uint8_t>(ChannelField::Vol)].value = voice::VOL_MAX;
            }
            result.insert(result.end(), fieldDefaults.begin(), fieldDefaults.end());
            break;
        }
        case Page::Oscillator:
        {
            auto fieldDefaults = loadFieldDefaults(voiceIndex, page, oscInfo);
            result.insert(result.end(), fieldDefaults.begin(), fieldDefaults.end());
            break;
        }
        case Page::Filter:
        {
            auto fieldDefaults = loadFieldDefaults(voiceIndex, page, filterInfo);
            result.insert(result.end(), fieldDefaults.begin(), fieldDefaults.end());
            break;
        }
        case Page::Envelope:
        {
            auto fieldDefaults = loadFieldDefaults(voiceIndex, page, envInfo);
            result.insert(result.end(), fieldDefaults.begin(), fieldDefaults.end());
            break;
        }
        case Page::Tuning:
        {
            auto fieldDefaults = loadFieldDefaults(voiceIndex, page, tuningInfo);
            result.insert(result.end(), fieldDefaults.begin(), fieldDefaults.end());
            break;
        }
        case Page::FilterRLFO:
        case Page::FilterCLFO:
        case Page::AmpLFO:
        case Page::PitchLFO:
        {
            auto fieldDefaults = loadFieldDefaults(voiceIndex, page, lfoInfo);
            result.insert(result.end(), fieldDefaults.begin(), fieldDefaults.end());
            break;
        }
        default:
            break;
        }
    }

    return result;
}

template <size_t N>
FieldUpdateList Presets::loadFieldDefaults(
    uint8_t voiceIndex,
    Page page,
    const FieldInfo (&fieldInfoArray)[N])
{
    FieldUpdateList result;

    for (uint8_t fieldIndex = 0; fieldIndex < N; ++fieldIndex)
    {
        const FieldInfo &info = fieldInfoArray[fieldIndex];

        FieldUpdate update = {
            .voiceIndex = voiceIndex,
            .pageByte = static_cast<uint8_t>(page),
            .field = fieldIndex,
            .value = info.defaultValue,
        };

        result.push_back(update);
    }

    return result;
}
