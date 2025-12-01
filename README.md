# TeensySynth

A Teensy 4.1-based synthesizer in Doepfer Eurorack format with a Norns Shield-style graphical GUI.

## Features

- **Norns-style GUI**: Minimalist, graphical interface inspired by Monome Norns
- **4-Script Multitasking**: Run up to 4 SuperCollider-compatible scripts simultaneously
- **Simple Controls**: 2 buttons (OK/Back) and a potentiometer for scrolling
- **ILI9341 Display**: 320x240 TFT display for rich visual feedback
- **Eurorack Format**: Designed for Doepfer Eurorack standard

## Hardware Requirements

- Teensy 4.1 (600MHz ARM Cortex-M7)
- ILI9341 2.8" TFT Display (320x240, SPI)
- 2 momentary push buttons
- 1 rotary encoder with push button (DEBO ENCODER)
- Eurorack power supply (±12V)

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

## Script Compatibility

TeensySynth is designed to run scripts compatible with the Norns ecosystem:
- Lua scripts (`.lua`)
- SuperCollider scripts (`.scd`)

Place scripts in the `/scripts` directory on the SD card.

## License

MIT License
