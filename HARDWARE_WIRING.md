# TeensySynth Hardware Wiring Guide

## Pin Assignment Summary

### Teensy 4.1 Pin Usage
| Pin | Function | Component | Notes |
|-----|----------|-----------|-------|
| 2 | Button | OK Button | Active LOW (pull-up) |
| 3 | Button | BACK Button | Active LOW (pull-up) |
| 4 | Encoder | CLK | DEBO encoder |
| 5 | Encoder | DT | DEBO encoder |
| 6 | Encoder | SW | Switch (also acts as OK) |
| 8 | Display | TFT_RST | ILI9341 Reset |
| 9 | Display | TFT_DC | ILI9341 Data/Command |
| 10 | Display | TFT_CS | ILI9341 Chip Select |
| 11 | Display | TFT_MOSI | SPI Data |
| 12 | Display | TFT_MISO | SPI Data |
| 13 | Display | TFT_SCLK | SPI Clock |
| 18 | I2C | SDA | Shared for both DACs |
| 19 | I2C | SCL | Shared for both DACs |

## Component Wiring Details

### 1. ILI9341 2.8" TFT Display
```
Display Pin    Teensy 4.1
-----------    ----------
VCC     →      3.3V
GND     →      GND
CS      →      Pin 10
RESET   →      Pin 8
DC      →      Pin 9
SDI/MOSI→      Pin 11
SCK     →      Pin 13
LED     →      3.3V (backlight)
SDO/MISO→      Pin 12
```

### 2. DEBO Rotary Encoder
```
Encoder Pin    Teensy 4.1
-----------    ----------
CLK     →      Pin 4
DT      →      Pin 5
SW      →      Pin 6
+       →      3.3V
GND     →      GND
```

**Important:** DEBO encoder generates 1 state transition per detent (not 4 like standard encoders).

### 3. Buttons
```
Button    Connection
------    ----------
OK        Pin 2 to GND (momentary, active low)
BACK      Pin 3 to GND (momentary, active low)
```

Both buttons use internal pull-up resistors. Pressing button connects pin to GND.

### 4. MCP4725 DAC #1 (CV Output) - Address 0x60
```
MCP4725 Pin    Teensy 4.1    Notes
-----------    ----------    -----
VDD     →      5V            Must be 5V for 0-5V range
GND     →      GND           
SDA     →      Pin 18        I2C Data (shared)
SCL     →      Pin 19        I2C Clock (shared)
A0      →      GND           Sets address to 0x60
OUT     →      TRRS Left     CV output 0-5V
```

### 5. MCP4725 DAC #2 (Gate Output) - Address 0x61
```
MCP4725 Pin    Teensy 4.1    Notes
-----------    ----------    -----
VDD     →      5V            Must be 5V for 0-5V range
GND     →      GND           
SDA     →      Pin 18        I2C Data (shared with DAC #1)
SCL     →      Pin 19        I2C Clock (shared with DAC #1)
A0      →      VDD (5V)      Sets address to 0x61
OUT     →      TRRS Ring     Gate output 0V/5V
```

**Critical:** The A0 pin determines the I2C address:
- A0 → GND = Address 0x60 (CV DAC)
- A0 → VDD = Address 0x61 (Gate DAC)

### 6. TRRS Jack Output (Eurorack CV/Gate)
```
TRRS Pin       Connection        Signal
--------       ----------        ------
Sleeve  →      GND               Common ground
Left    →      DAC #1 OUT        CV output (0-5V analog)
Ring    →      DAC #2 OUT        Gate output (0V/5V digital)
Right   →      (unused)          Reserved for future
```

## I2C Bus Configuration

Both MCP4725 DACs share the same I2C bus (SDA=18, SCL=19) but use different addresses:
- **DAC #1 (CV):** Address 0x60 (A0 pin → GND)
- **DAC #2 (Gate):** Address 0x61 (A0 pin → VDD)

## Power Requirements

- **Teensy 4.1:** USB power (5V) or external 5V via VIN
- **Display:** 3.3V from Teensy (draws ~30-50mA)
- **Encoder:** 3.3V from Teensy (minimal current)
- **Buttons:** No power (uses pull-ups)
- **MCP4725 #1:** 5V (critical for full 0-5V output range)
- **MCP4725 #2:** 5V (critical for full 0-5V gate levels)

**Important:** Powering MCP4725 with 3.3V will limit output to 0-3.3V, which is insufficient for Eurorack CV/Gate standards.

## Eurorack CV/Gate Standards

### CV (Control Voltage)
- Standard: 1 V/octave
- Reference: C4 (MIDI 60) = 2.0V
- Range: 0-5V (covers 5 octaves)
- Used for pitch control

### Gate
- High: 5V (note on)
- Low: 0V (note off)
- Typical pulse width: 50% of step duration
- Used for triggering envelopes

## Troubleshooting

### Display not working
- Check SPI connections (pins 10-13)
- Verify 3.3V power
- Check orientation setting in code: `setRotation(3)`

### Encoder too sensitive/insensitive
- `ENC_STEPS_PER_NOTCH = 1` for DEBO encoder
- Standard encoders may need `= 4`

### MCP4725 not detected
- Verify I2C connections (SDA=18, SCL=19)
- Check A0 pin wiring (GND for 0x60, VDD for 0x61)
- Confirm 5V power to VDD pin
- Use I2C scanner sketch to detect addresses

### CV output voltage too low
- MCP4725 VDD must be 5V (not 3.3V)
- Check voltage at MCP4725 OUT pin with multimeter
- Verify TRRS jack wiring (sleeve must be grounded)

### Gate not triggering
- Check second MCP4725 A0 pin (must be VDD for 0x61 address)
- Verify gate DAC initialization in serial monitor
- Test gate voltage with multimeter (should toggle 0V/5V)

## Serial Monitor Output

At startup, you should see:
```
TeensySynth Initializing...
LFO: Initialized successfully with MCP4725 DAC
     CV output available on VOUT pin (connect to TRRS jack)
SEQ: CV DAC initialized
SEQ: Gate DAC initialized
ScriptManager: Initialized
TeensySynth Ready!
```

If DACs are not detected, check wiring and addresses.
