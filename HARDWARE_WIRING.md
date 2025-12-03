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

### 4. DAC8568 - 8-Channel 16-bit DAC (CV Outputs)
```
DAC8568 Pin    Teensy 4.1    Notes
-----------    ----------    -----
VDD     →      5V            Power supply
VSS     →      GND           Ground
VREFIN  →      5V            External reference (for 0-5V range)
VREFOUT →      (NC)          Leave unconnected (or 100nF to GND)
SCLK    →      Pin 13        SPI Clock (shared with display/touch)
DIN     →      Pin 11        SPI MOSI (shared with display/touch)
SYNC    →      Pin 14        SPI CS (dedicated for DAC)
LDAC    →      GND           Tied low for immediate updates
CLR     →      5V            Tied high (never clear outputs)
DOUT    →      (NC)          Leave unconnected
VOUTA   →      CV Out 1      Channel 0: Step 1 CV / LFO output
VOUTB   →      CV Out 2      Channel 1: Step 2 CV
VOUTC   →      CV Out 3      Channel 2: Step 3 CV
VOUTD   →      CV Out 4      Channel 3: Step 4 CV
VOUTE   →      CV Out 5      Channel 4: Step 5 CV
VOUTF   →      CV Out 6      Channel 5: Step 6 CV
VOUTG   →      CV Out 7      Channel 6: Step 7 CV
VOUTH   →      CV Out 8      Channel 7: Step 8 CV / Gate
```

**DAC8568 Features:**
- 16-bit resolution (65,535 steps) vs 12-bit on MCP4725 (4,096 steps)
- 8 independent channels for polyphonic output
- SPI interface (faster than I2C)
- All channels update simultaneously
- Shared SPI bus with display and touch (uses separate CS on Pin 14)

**Polyphonic Sequencing:**
The Poliquencer outputs all 8 steps continuously on separate channels, enabling:
- Polyphonic patches (8-voice polyphony)
- Multi-destination modulation (8 parameters simultaneously)
- Parallel sequencing patterns

### 5. CV Output Connections
Each DAC8568 channel provides 0-5V CV output:
```
Channel    Output        Use Case
-------    ------        --------
0 (A)      CV Out 1      Poliquencer Step 1 / LFO primary
1 (B)      CV Out 2      Poliquencer Step 2
2 (C)      CV Out 3      Poliquencer Step 3
3 (D)      CV Out 4      Poliquencer Step 4
4 (E)      CV Out 5      Poliquencer Step 5
5 (F)      CV Out 6      Poliquencer Step 6
6 (G)      CV Out 7      Poliquencer Step 7
7 (H)      CV Out 8      Poliquencer Step 8 / Gate
```

Connect to TRS/TRRS jacks, banana jacks, or directly to Eurorack patch cables.

## SPI Bus Configuration

The SPI bus (MOSI=11, MISO=12, SCK=13) is shared between three devices:
- **ILI9341 Display:** CS=10
- **XPT2046 Touch:** CS=7
- **DAC8568:** CS=14

Each device has its own chip select pin for bus arbitration. Only one device is active at a time.

## Power Requirements

- **Teensy 4.1:** USB power (5V) or external 5V via VIN
- **Display:** 3.3V from Teensy (draws ~30-50mA)
- **Touch Controller:** 3.3V from Teensy (minimal current)
- **Encoder:** 3.3V from Teensy (minimal current)
- **Buttons:** No power (uses pull-ups)
- **DAC8568:** 5V (critical for full 0-5V output range)

**Important:** DAC8568 VDD and VREFIN must both be 5V for proper 0-5V output range. Using 3.3V will limit output to 0-3.3V, insufficient for Eurorack standards.

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
- Check SPI connections (MOSI=11, MISO=12, SCK=13, CS=10)
- Verify 3.3V power to display
- Check orientation setting in code: `setRotation(3)`
- Ensure TFT_CS (Pin 10) is properly connected

### Touch not responding
- Verify XPT2046 CS pin (Pin 7) is connected
- Check that touch shares SPI bus with display (MOSI=11, MISO=12, SCK=13)
- Calibration values in config.h may need adjustment for your specific display
- T_IRQ can be left unconnected (firmware uses polling mode)

### Encoder too sensitive/insensitive
- `ENC_STEPS_PER_NOTCH = 1` for DEBO encoder
- Standard encoders may need `= 4`

### DAC8568 not detected
- Verify SPI connections (DIN=11, SCLK=13, SYNC=14)
- Confirm 5V power to both VDD and VREFIN
- Check DAC_CS (Pin 14) is properly connected
- Monitor serial output for "DAC8568 initialized successfully" message
- Verify LDAC is tied to GND and CLR is tied to VDD

### CV output voltage too low
- DAC8568 VDD must be 5V (not 3.3V)
- VREFIN must also be 5V for full 0-5V range
- Check voltages at VOUT pins with multimeter
- Verify output jack wiring (ground connections)

### No CV output on some channels
- Check all VOUT pins are properly connected
- Use serial monitor to verify which script is loaded
- Test each channel individually with multimeter
- DAC8568 outputs persist even when script is stopped

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
