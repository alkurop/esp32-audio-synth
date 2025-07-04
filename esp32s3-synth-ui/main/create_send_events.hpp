#pragma once
#include "protocol.hpp"
#include <cstdint>
using namespace protocol;

inline EventList createBpmEventList(uint16_t bpm)
{
    EventList events;
    events.reserve(1);
    protocol::Event e;
    e.type = protocol::EventType::BpmFromMidi;
    e.midiBpm = bpm;
    events.push_back(e);
    return events;
};

inline EventList createNoteEventList(const MidiNoteEvent &note)
{
    EventList events;
    events.reserve(1);
    protocol::Event e;
    e.type = protocol::EventType::MidiNote;
    e.note = note;
    events.push_back(e);
    return events;
};


inline EventList createFileUpdateEventList(const FieldUpdateList &updates)
{
    protocol::Event e;
    e.type = protocol::EventType::FieldUpdate;
    e.fields = updates;

    // 2) Make a one‐element EventList
    EventList events;
    events.reserve(1);
    events.push_back(std::move(e));
    return events;
};
