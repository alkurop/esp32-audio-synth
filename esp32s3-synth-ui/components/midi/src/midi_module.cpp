#include "midi_module.hpp"
#define TAG "MidiModule"

using namespace midi_module;

static void midiReader(void *arg)
{
  auto midiModule = static_cast<MidiModule *>(arg);
  uint8_t packet[4];
  bool read = false;
  for (;;)
  {
    vTaskDelay(1);
    // Check if module is mounted
    while (tud_midi_mounted())
    {
      read = tud_midi_packet_read(packet);
      // Try to read a MIDI packet.
      if (read)
      {
        // If a packet is available, invoke the callback.
        midiModule->readCallback(packet);
      }
      else
      {
        vTaskDelay(1);
      }
    }
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

  xTaskCreate(midiReader, "midiReader", 4 * 1024, this,
              configMAX_PRIORITIES - 1, &handle);
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
