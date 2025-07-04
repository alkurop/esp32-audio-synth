#pragma once

#include "audio_config.hpp"
#include "menu_struct.hpp"
#include "events.hpp"

using namespace midi_module;
namespace protocol
{

    static constexpr int8_t AUTOSAVE_SLOT = -1;

    template <class... Ts>
    struct overloaded : Ts...
    {
        using Ts::operator()...;
    };
    template <class... Ts>
    overloaded(Ts...) -> overloaded<Ts...>;
    struct __attribute__((packed)) FieldUpdate
    {
        uint8_t voiceIndex;
        uint8_t pageByte;
        uint8_t field;
        int16_t value;
    };

    struct IncomingMessage
    {
        uint8_t *buffer;
        size_t length;
    };

    enum class EventType : uint8_t
    {
        MidiNote = 0x01,
        FieldUpdate = 0x02,
        BpmFromMidi = 0x03,
    };
    using FieldUpdateList = std::vector<FieldUpdate>;
    struct Event
    {
        EventType type;
        MidiNoteEvent note;     // valid if type==MidiNote
        FieldUpdateList fields; // valid if type==FieldUpdate
        uint16_t midiBpm; // valid if type==MidiBpm
    };

    using EventList = std::vector<Event>;
    using UpdateCallback = std::function<void(EventList)>;
    using FieldUpdateCallback = std::function<void(FieldUpdateList)>;
}
