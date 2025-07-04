#pragma once

#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <functional>
#include <tinyusb.h>
#include <tusb_cdc_acm.h>
#include <tusb_console.h>
#include "midi_parser.hpp"

/** TinyUSB descriptors **/
// Interface numbers
#define ITF_NUM_CDC 0      // CDC Comm
#define ITF_NUM_CDC_DATA 1 // CDC Data
#define ITF_NUM_MIDI 2     // MIDI
#define ITF_COUNT 3        // total number of interfaces

// Endpoint numbers
#define EPNUM_CDC_NOTIF 0x81
#define EPNUM_CDC_OUT 0x02
#define EPNUM_CDC_IN 0x82

#define EPNUM_MIDI_OUT 0x03
#define EPNUM_MIDI_IN 0x83

namespace midi_module
{
    using MidiReadCallback = std::function<void(const uint8_t packet[4])>;

    // USB Endpoint numbers
    enum UsbEndpoints
    {
        // Available USB Endpoints: 5 IN/OUT EPs and 1 IN EP
        EP_EMPTY = 0,
        EPNUM_MIDI,
    };

    /*! Enumeration of MIDI types */
    enum MidiType : uint8_t
    {

        ControlChange = 0xB0, ///< Channel Message - Control Change / Channel Mode
                              ///< send(ControlChange, inControlNumber,
                              ///< inControlValue, inChannel);

    };

    class MidiModule
    {
    private:
        TaskHandle_t handle;
        uint8_t cable_num;
        uint8_t channel;

    public:
        MidiModule();
        void init(MidiReadCallback readCallback);
        bool isMounted();
        MidiReadCallback readCallback;
        void sendValue(uint8_t program, uint8_t value);
    };

    static const uint8_t s_midi_cdc_cfg_desc[] = {
        // Configuration descriptor: config number, interface count, string index,
        // total length, attributes, power (mA)
        TUD_CONFIG_DESCRIPTOR(
            1, ITF_COUNT, 0,
            (TUD_CONFIG_DESC_LEN + TUD_CDC_DESC_LEN + TUD_MIDI_DESC_LEN),
            TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, 100),

        // CDC: Interface number, string index, EP notification, EP size, EP
        // interval, EP OUT, EP IN
        TUD_CDC_DESCRIPTOR(
            ITF_NUM_CDC, 3, EPNUM_CDC_NOTIF, 8, EPNUM_CDC_OUT,
            EPNUM_CDC_IN, 64),

        // MIDI: Interface number, string index, EP OUT, EP IN, EP size
        TUD_MIDI_DESCRIPTOR(ITF_NUM_MIDI, 4, EPNUM_MIDI_OUT, EPNUM_MIDI_IN, 64),
    };
}; // namespace midi_module
