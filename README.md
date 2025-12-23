# Polyphonion

A Teensy 4.1-based synthesizer in Doepfer Eurorack format with a Norns Shield-style graphical GUI.

## Features

- **Norns-style GUI**: Minimalist, graphical interface inspired by Monome Norns
- **4-Script Multitasking**: Run up to 4 concurrent scripts in parallel slots
- **Built-in LFO**: Low-frequency oscillator with CV output (0-5V)
- **CV Output**: MCP4725 12-bit DAC for precise Eurorack CV generation
- **Simple Controls**: 2 buttons (OK/Back) and rotary encoder for navigation
- **ILI9341 Display**: 320x240 TFT display for rich visual feedback
- **Eurorack Format**: Designed for Doepfer Eurorack standard

## Hardware Requirements

- Teensy 4.1 (600MHz ARM Cortex-M7)
- ILI9341 2.8" TFT Display (320x240, SPI)
- 2 momentary push buttons
- 1 rotary encoder with push button (DEBO ENCODER)
- 2x MCP4725 12-bit I2C DACs for CV/Gate output
- Eurorack power supply (±12V)

## Documentation

- **[MCP4725 Quick Start](MCP4725_QUICK_START.md)** - Print this! One-page wiring reference
- **[MCP4725 Setup Guide](MCP4725_SETUP_GUIDE.md)** - Complete wiring & configuration guide
- **[MCP4725 Address Guide](MCP4725_ADDRESS_GUIDE.md)** - Deep dive on I2C addressing
- **[Hardware Wiring](HARDWARE_WIRING.md)** - Full pin assignments and troubleshooting
- **[Design Principles](docs/DESIGN_PRINCIPLES.md)** - Architecture and design goals

### Pin Connections

#### ILI9341 Display
| Display Pin | Teensy 4.1 Pin | Notes |
|-------------|----------------|-------|
| SDO (MISO)  | 12            | Hardware SPI |
| LED         | 3.3V          | Backlight (always on) |
| SCK         | 13            | Hardware SPI |
| SDI (MOSI)  | 11            | Hardware SPI |
| DC          | 9             | Data/Command select |
| RESET       | 8             | Reset |
| CS          | 10            | Chip Select |
| GND         | GND           | Ground |
| VCC         | 3.3V          | **3.3V ONLY** |

#### Buttons & Encoder
| Component | Teensy Pin | Notes |
|-----------|------------|-------|
| OK Button | 2          | Momentary, pull-up |
| Back Button | 3        | Momentary, pull-up |
| Encoder CLK | 4        | Rotary encoder A |
| Encoder DT  | 5        | Rotary encoder B |
| Encoder SW  | 6        | Encoder push button |
| Encoder +   | 3.3V     | Power |
| Encoder GND | GND      | Ground |

#### MCP4725 DACs (CV and Gate Output)

**DAC #1 (CV Output - Address 0x60):**
| MCP4725 Pin | Teensy 4.1 Pin | Notes |
|-------------|----------------|-------|
| VCC         | 5V            | For full 0-5V CV range |
| GND         | GND           | Ground |
| SDA         | 18            | I2C Data (shared) |
| SCL         | 19            | I2C Clock (shared) |
| OUT         | TRRS Left     | 0-5V Eurorack CV output |

**DAC #2 (Gate Output - Address 0x61):**
| MCP4725 Pin | Teensy 4.1 Pin | Notes |
|-------------|----------------|-------|
| VCC         | 5V            | For 5V gate signals |
| GND         | GND           | Ground |
| SDA         | 18            | I2C Data (shared) |
| SCL         | 19            | I2C Clock (shared) |
| OUT         | TRRS Ring     | Gate output (0V/5V) |

**Important:** You need two MCP4725 modules with different I2C addresses. Most modules are fixed at 0x60, so you'll need to source a second module with address 0x61 (or 0x62, and update `config.h`).

## Building

This project uses PlatformIO. To build:

```bash
# Install PlatformIO CLI if not already installed
pip install platformio

# Build the firmware
pio run

# Upload to Teensy
pio run --target upload
```

## Controls

- **Rotary Encoder**: Turn to scroll through menus and options
- **Encoder Button**: Alternative select (same as OK button)
- **OK Button**: Select/confirm
- **Back Button**: Go back/cancel

## User Interface

1. **Welcome Screen**: Displays on startup with project branding
2. **Main Menu**: Access Scripts, Settings, and About
3. **Scripts**: Select and load up to 4 concurrent scripts
4. **Settings**: Configure audio output, MIDI, display brightness
5. **About**: System information

## Built-in Scripts

### LFO (Low-Frequency Oscillator)
The built-in LFO script provides CV modulation for Eurorack systems:
- **Waveforms**: Sine, Triangle, Square, Sawtooth
- **Frequency Range**: 0.01 Hz to 100 Hz
- **Output**: 0-5V CV via MCP4725 DAC
- **Real-time Display**: Shows frequency, level, and waveform

**Usage:**
1. Go to **SCRIPTS** menu
2. Select **Slot 1**
3. Choose **LFO** from the library
4. The LFO starts running immediately with CV output

### Future Scripts (Planned)
- **Sequencer**: CV/Gate step sequencer
- **Envelope**: ADSR envelope generator
- **Clock**: Clock divider/multiplier

## Script Development

Future versions will support custom scripts compatible with the Norns ecosystem:
- Lua scripts (`.lua`)
- SuperCollider scripts (`.scd`)

See the `scripts/` directory for more information.

## License

MIT License
