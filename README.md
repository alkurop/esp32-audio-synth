# ESP32-S3 Synthesizer

This repository contains the source code for a dual-chip synthesizer project on the Espressif ESP32-S3 platform. Two ESP32-S3 MCUs share responsibilities:

1. **Audio MCU** (`esp32s3-synth`): Real-time sound generation.
2. **UI MCU** (`esp32s3-synth-ui`): Display and user controls.

## Module Overview

| Module                   | Location                                 | Description                                        |
| ------------------------ | ---------------------------------------- | -------------------------------------------------- |
| **Button Control**       | `common/button`                          | Debounced button input abstraction                 |
| **Potentiometer**        | `common/knob`                            | ADC-based potentiometer reading                    |
| **Rotary Encoder**       | `common/rotary_pcnt`                     | Quadrature encoder processing                      |
| **Inter-chip Protocol**  | `common/protocol`                        | Data packet definitions and serialization          |
| **Envelope Generator**   | `esp32s3-synth/components/envelope`      | ADSR envelope implementation                       |
| **Filter**               | `esp32s3-synth/components/filter`        | Multimode filter algorithms                        |
| **LFO**                  | `esp32s3-synth/components/lfo`           | Low-frequency oscillator for modulation            |
| **Lookup Tables**        | `esp32s3-synth/components/lookup`        | Waveshaping and waveform data                      |
| **MIDI & Comm Receiver** | `esp32s3-synth/components/receiver`      | Parses incoming MIDI and UI messages               |
| **Core Sound Engine**    | `esp32s3-synth/components/sound`         | Oscillator voices, noise generator, voice mixing   |
| **Waveform Switch**      | `esp32s3-synth/components/switch`        | Runtime waveform selector logic                    |
| **Synth Main**           | `esp32s3-synth/main`                     | Initialization, FreeRTOS tasks, I2S audio pipeline |
| **UI Cache**             | `esp32s3-synth-ui/components/cache_comp` | Local caching of UI state                          |
| **Display Driver**       | `esp32s3-synth-ui/components/display`    | OLED/TFT initialization and graphics routines      |
| **Menu System**          | `esp32s3-synth-ui/components/menu`       | Screen navigation and parameter editing            |
| **UI MIDI Handler**      | `esp32s3-synth-ui/components/midi`       | Reads MIDI CC (e.g., mod wheel, pitch bend)        |
| **Preset Storage**       | `esp32s3-synth-ui/components/presets`    | Save/load patches in flash                         |
| **UI Comm Sender**       | `esp32s3-synth-ui/components/sender`     | Sends parameter updates to audio MCU               |
| **UI Data Store**        | `esp32s3-synth-ui/components/store`      | Central state management for UI parameters         |
| **UI Main**              | `esp32s3-synth-ui/main`                  | Initialization, event loop, task setup             |

## Repository Structure

```
common/               # Shared input drivers and inter-chip protocol
esp32s3-synth/        # Audio MCU firmware
esp32s3-synth-ui/     # UI MCU firmware
```

### `/common`

- `button/`       – Push-button abstraction
- `knob/`         – Potentiometer driver
- `rotary_pcnt/`  – Rotary encoder handling
- `protocol/`     – Shared data packet definitions

### `/esp32s3-synth`

- `components/`   – DSP building blocks: envelope, filter, LFO, lookup, receiver, sound, switch
- `main/`         – Application entry, FreeRTOS tasks, I2S audio setup
- `scripts/`      – Utility scripts (build, testing)
- `CMakeLists.txt` & `sdkconfig.defaults` etc. – ESP-IDF project files

### `/esp32s3-synth-ui`

- `components/`   – UI modules: cache\_comp, display, menu, midi, presets, sender, store
- `main/`         – Initialization, UI event loop
- `scripts/`      – Build and helper scripts
- `CMakeLists.txt` & `sdkconfig.defaults` etc. – ESP-IDF project files

## Prerequisites

- **Toolchain**: ESP-IDF v5.0+ installed and sourced
- **Python**: 3.8+ (for ESP-IDF scripts)
- **Hardware**:
  - Two ESP32-S3 dev boards
  - I2S audio codec or DAC
  - OLED/TFT display, buttons, encoders, potentiometers

## Getting Started

1. **Clone**
   ```bash
   git clone https://github.com/alkurop/esp32-synth-joined.git
   cd esp32-synth-joined
   ```
2. **Setup ESP-IDF** per Espressif's guide.
3. **Build Common**
   ```bash
   cd common
   idf.py build
   ```
4. **Build & Flash Audio MCU**
   ```bash
   cd ../esp32s3-synth
   idf.py set-target esp32s3
   idf.py flash
   ```
5. **Build & Flash UI MCU**
   ```bash
   cd ../esp32s3-synth-ui
   idf.py set-target esp32s3
   idf.py flash
   ```
 
## Configuration

- **Common Audio Settings**: `common/protocol/include/audio_config.hpp`

- **Synth Engine Config**: `esp32s3-synth/main/synth_config.hpp`

- **UI Global Config**: `esp32s3-synth-ui/main/config.hpp`


## Contributing

1. Fork → branch → commit → PR.
2. Follow existing code style.

## License

MIT. See [LICENSE](./LICENSE).

