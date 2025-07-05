#include "midi_module.hpp"
#include "esp_log.h"

#define TAG "MidiModule"

using namespace midi_module;

static void midiReader(void *arg)
{
  auto midiModule = static_cast<MidiModule *>(arg);
  uint8_t packet[4];
  bool read = false;
  for (;;)
    {
        // 1) Service TinyUSB so it can receive new USB packets
        tud_task();

        // 2) Only delay if the device isn’t mounted
        if (!tud_midi_mounted()) {
            vTaskDelay(pdMS_TO_TICKS(5));
            continue;
        }

        // 3) Drain every pending packet in one go
        while (tud_midi_available())  // returns count of pending packets :contentReference[oaicite:0]{index=0}
        {
            tud_midi_packet_read(packet);  // always succeeds inside this loop :contentReference[oaicite:1]{index=1}
            midiModule->readCallback(packet);
        }

        // 4) Let other tasks run briefly before next poll
        taskYIELD();
    }
}

MidiModule::MidiModule()
    : handle(NULL), cable_num(0x00), channel(0x00) {};

void MidiModule::init(MidiReadCallback readCallback)
{
  this->readCallback = readCallback;
  static const char *s_str_desc[] = {
      (char[]){0x09, 0x04},     // Language (English)
      CONFIG_MIDI_COMPANY_NAME, // Manufacturer
      CONFIG_MIDI_PRODUCT_NAME, // Product
      CONFIG_MIDI_DEVICE_ID,    // Serial number
      CONFIG_MIDI_PRODUCT_NAME};
  tinyusb_config_t const midi_config = {
      .device_descriptor = NULL, // If device_descriptor is NULL,
                                 // tinyusb_driver_install() will use Kconfig
      .string_descriptor = s_str_desc,
      .string_descriptor_count = sizeof(s_str_desc) / sizeof(s_str_desc[0]),
      .external_phy = false,
      .configuration_descriptor = s_midi_cdc_cfg_desc,
      .self_powered = false, // Initialize self_powered explicitly
      .vbus_monitor_io = 0,  // or another appropriate value
  };

  tinyusb_config_cdcacm_t acm_cfg = {
      .usb_dev = TINYUSB_USBDEV_0,   // Required: valid USB device number
      .cdc_port = TINYUSB_CDC_ACM_0, // CDC port index (usually 0)
      .rx_unread_buf_sz = 64,        // Buffer size for incoming data (optional)
      .callback_rx = NULL,           // Callback for RX (optional)
      .callback_rx_wanted_char = NULL,
      .callback_line_state_changed = NULL,
      .callback_line_coding_changed = NULL,
  };
  tusb_cdc_acm_init(&acm_cfg);
  tinyusb_driver_install(&midi_config);
  esp_tusb_init_console(TINYUSB_CDC_ACM_0); // log to usb

  xTaskCreatePinnedToCore(midiReader, "midiReader", 8 * 1024, this,
              configMAX_PRIORITIES - 1, &handle, 1);
};

void MidiModule::sendValue(uint8_t program, uint8_t value)
{
  if (tud_midi_mounted())
  {
    uint8_t event = (ControlChange | this->channel);
    uint8_t data[3] = {event, program, value};
    tud_midi_stream_write(cable_num, data, 3);
    ESP_LOGI(TAG, "Sent midi event %d program %d value %d", event, program, value);
  }
  else
  {
    ESP_LOGI(TAG, "Not mounted");
  }
};
