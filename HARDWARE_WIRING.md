# Polyphonion Hardware Wiring Guide

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
| 7 | Touch | T_CS | XPT2046 Chip Select |
| 11 | Touch | T_DIN | Shared with TFT MOSI |
| 12 | Touch | T_DO | Shared with TFT MISO |
| 13 | Touch | T_CLK | Shared with TFT SCLK |
| 18 | I2C | SDA | Shared for both DACs |
| 19 | I2C | SCL | Shared for both DACs |

## Component Wiring Details

### 1. ILI9341 2.8" TFT Display with XPT2046 Touch
```
Display Pin    Teensy 4.1    Notes
-----------    ----------    -----
VCC     →      3.3V          Display power
GND     →      GND           Ground
CS      →      Pin 10        Display chip select
RESET   →      Pin 8         Display reset
DC      →      Pin 9         Display data/command
SDI/MOSI→      Pin 11        SPI data (shared with touch)
SCK     →      Pin 13        SPI clock (shared with touch)
LED     →      3.3V          Backlight power
SDO/MISO→      Pin 12        SPI data (shared with touch)
T_CLK   →      Pin 13        Touch clock (shared with display)
T_CS    →      Pin 7         Touch chip select
T_DIN   →      Pin 11        Touch data in (shared with display MOSI)
T_DO    →      Pin 12        Touch data out (shared with display MISO)
T_IRQ   →      Not connected Touch uses polling mode
```

**Note:** The touch controller (XPT2046) shares the SPI bus with the display but has its own chip select (Pin 7).

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
VCC     →      5V            Must be 5V for 0-5V range
GND     →      GND           
SDA     →      Pin 18        I2C Data (shared)
SCL     →      Pin 19        I2C Clock (shared)
OUT     →      TRRS Left     CV output 0-5V
```

### 5. MCP4725 DAC #2 (Gate Output) - Address 0x61
```
MCP4725 Pin    Teensy 4.1    Notes
-----------    ----------    -----
VCC     →      5V            Must be 5V for 0-5V range
GND     →      GND           
SDA     →      Pin 18        I2C Data (shared with DAC #1)
SCL     →      Pin 19        I2C Clock (shared with DAC #1)
OUT     →      TRRS Ring     Gate output 0V/5V
```

**Important:** Most MCP4725 breakout boards have fixed I2C addresses set by the manufacturer:
- Standard boards: Address 0x60 or 0x62 (check your module's documentation)
- If you need two DACs, you must purchase modules with **different addresses**
- Common combinations: 0x60 + 0x61, or 0x60 + 0x62
- Some modules have solder jumpers to change the address

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

Both MCP4725 DACs share the same I2C bus (SDA=18, SCL=19) but must have different addresses:
- **DAC #1 (CV):** Address 0x60 (default on most modules)
- **DAC #2 (Gate):** Address 0x61 (or 0x62 if 0x61 unavailable)

**Note:** Standard MCP4725 breakout boards have fixed addresses. You need to purchase two modules with different addresses, or use modules with solder jumpers to change the address.

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
- Confirm 5V power to VCC pin
- Check that modules have different addresses (use I2C scanner to verify)
- Ensure you're using addresses 0x60 and 0x61 (or update config.h if different)
- Use I2C scanner sketch to detect actual addresses on your modules

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
Polyphonion Initializing...
LFO: Initialized successfully with MCP4725 DAC
     CV output available on VOUT pin (connect to TRRS jack)
SEQ: CV DAC initialized
SEQ: Gate DAC initialized
ScriptManager: Initialized
Polyphonion Ready!
```

If DACs are not detected, check wiring and addresses.
