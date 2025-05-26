#include "midi_parser.hpp"
#include "tag.hpp"

using namespace midi_module;

MidiParser::MidiParser() = default;

void MidiParser::feed(const uint8_t packet[4])
{
    uint8_t status = packet[1];
    uint8_t type = status & 0xF0;
    uint8_t channel = status & 0x0F;

    MidiMessageType message = static_cast<MidiMessageType>(status);
    MidiMessageType baseType = static_cast<MidiMessageType>(type);

    switch (message)
    {
    case MidiMessageType::Start:
    case MidiMessageType::Continue:
    case MidiMessageType::Stop:
        parseTransportCommand(packet);
        break;

    case MidiMessageType::TimingClock:
        parseTimingClock(packet);
        break;

    case MidiMessageType::SongPosition:
        parseSongPosition(packet);
        break;

    default:
        switch (baseType)
        {
        case MidiMessageType::ControlChange:
            parseControllerChange(packet);
            break;

        case MidiMessageType::NoteOn:
            parseNoteMessage(packet, true);
            break;

        case MidiMessageType::NoteOff:
            parseNoteMessage(packet, false);
            break;

        default:
            ESP_LOGI(TAG, "Unknown MIDI message: %s, %d, %d, %d, %d",
                     to_string(message), packet[0], packet[1], packet[2], packet[3]);
            break;
        }
        break;
    }
}

void MidiParser::parseTimingClock(const uint8_t packet[4])
{
    bpmCounter.onClockTick(esp_timer_get_time());
}

void MidiParser::parseControllerChange(const uint8_t packet[4])
{
    uint8_t status = packet[1];
    uint8_t channel = status & 0x0F;

    ControllerChange msg;
    msg.channel = channel;
    msg.controller = packet[2];
    msg.value = packet[3];

    if (msg.controller == static_cast<uint8_t>(MidiMessageType::Stop) && msg.value == 0)
    {
        bpmCounter.stop();
    }
    else if (msg.controller == static_cast<uint8_t>(MidiMessageType::Start) && msg.value > 0)
    {
        bpmCounter.start();
    }

    if (controllerCallback)
    {
        controllerCallback(msg);
    }
}
void MidiParser::parseSongPosition(const uint8_t packet[4])
{
    SongPosition msg;
    msg.position = static_cast<uint16_t>((packet[3] << 7) | packet[2]);

    if (songPositionCallback)
    {
        songPositionCallback(msg);
    }
}

void MidiParser::parseNoteMessage(const uint8_t packet[4], bool on)
{
    uint8_t status = packet[1];
    uint8_t channel = status & 0x0F;

    NoteMessage msg;
    msg.channel = channel;
    msg.note = packet[2];
    msg.velocity = packet[3];
    msg.on = on && msg.velocity > 0;

    if (noteMessageCallback)
    {
        noteMessageCallback(msg);
    }
}

void MidiParser::parseTransportCommand(const uint8_t packet[4])
{
    TransportCommand command;

    switch (packet[1])
    {
    case 0xFA:
        command = TransportCommand::Start;
        break;
    case 0xFB:
        command = TransportCommand::Continue;
        break;
    case 0xFC:
        command = TransportCommand::Stop;
        break;
    default:
        command = TransportCommand::Unknown;
        break;
    }

    if (command != TransportCommand::Unknown && transportCallback)
    {
        transportCallback(TransportEvent{command});
    }
}
