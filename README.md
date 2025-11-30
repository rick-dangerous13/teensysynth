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
- ILI9341 2.8" TFT Display (320x240)
- 2 momentary push buttons
- 1 potentiometer (10K linear)
- Eurorack power supply (±12V)

### Pin Connections

| Component | Teensy Pin |
|-----------|------------|
| TFT DC    | 9          |
| TFT CS    | 10         |
| TFT RST   | 8          |
| TFT MOSI  | 11         |
| TFT SCLK  | 13         |
| TFT MISO  | 12         |
| OK Button | 2          |
| Back Button | 3        |
| Scroll Pot | A0        |

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

- **Potentiometer**: Scroll through menus and options
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
