# Polyphonion Hardware Wiring Guide

## Quick Reference: MCP4725 I2C Addressing

**How to Address Two Separate DACs on the Same I2C Bus:**

The MCP4725 has ONE address pin (**A0**) that determines the I2C address:

| Configuration | A0 Pin Connection | I2C Address | Purpose |
|---------------|-------------------|-------------|---------|
| DAC #1        | Connected to **GND** | **0x60** | CV Output (Pitch) |
| DAC #2        | Connected to **VCC (5V)** | **0x61** | Gate / Modulation |

Both DACs share **SDA (Pin 18)** and **SCL (Pin 19)** — the I2C protocol automatically routes commands to the correct DAC based on address.

**Important:** The A0 pin must be explicitly connected (not floating). Floating pins cause the DAC to be unaddressable.

## Pin Assignment Summary

### Teensy 4.1 Pin Usage
| Pin | Function | Component | Notes |
|-----|----------|-----------|-------|
| 2 | Button | OK Button | Active LOW (pull-up) |
| 3 | Button | BACK Button | Active LOW (pull-up) |
| 4 | Encoder | CLK | DEBO encoder |
| 5 | Encoder | DT | DEBO encoder |
| 6 | Encoder | SW | Switch (also acts as OK) |
| 8 | Display | TFT_RST | ILI9488 Reset |
| 9 | Display | TFT_DC | ILI9488 Data/Command |
| 10 | Display | TFT_CS | ILI9488 Chip Select |
| 11 | Display | TFT_MOSI | SPI Data |
| 12 | Display | TFT_MISO | SPI Data |
| 13 | Display | TFT_SCLK | SPI Clock |
| 7 | Touch | T_CS | XPT2046 Chip Select |
| 11 | Touch | T_DIN | Shared with TFT MOSI |
| 12 | Touch | T_DO | Shared with TFT MISO |
| 13 | Touch | T_CLK | Shared with TFT SCLK |
| 16 | DAC | DAC_CS | DAC8568 Chip Select |
| 15 | Button | OK2 Button | Alternative OK (pull-up) |

## Component Wiring Details

### 1. ILI9488 3.5" TFT Display (Capacitive Touch - Not Connected)
```
Display Pin    Teensy 4.1    Notes
-----------    ----------    -----
VDD     →      3.3V          Display power
GND     →      GND           Ground
CS      →      Pin 10        Display chip select
RST     →      Pin 8         Display reset
D/C     →      Pin 9         Display data/command
SDI     →      Pin 11        SPI MOSI data
SCK     →      Pin 13        SPI clock
BL      →      3.3V          Backlight power
SDO     →      Pin 12        SPI MISO data
NC/3V3  →      Not connected
CTP_SDA →      Not connected Capacitive touch (not in use)
CTP_SCL →      Not connected Capacitive touch (not in use)
CTP_INT →      Not connected Capacitive touch (not in use)
CTP_RST →      Not connected Capacitive touch (not in use)
```

**Resolution:** 320 × 480 pixels (landscape orientation)
**Note:** Capacitive touch functionality not yet implemented. Touch pins can be added later if needed.

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

### 4. MCP4725 12-bit I2C DAC (CV Outputs)
**Current Configuration:** 2x MCP4725 DACs on I2C bus for CV output

#### Physical Wiring (Each MCP4725 Module)
**Your module pin labels (may vary by breakout board):**
```
MCP4725 Pin    Teensy 4.1    Notes
-----------    ----------    -----
OUT     →      TRRS Jack     CV Output (0-5V)
GND     →      GND           Ground
SCL     →      Pin 19        I2C Clock (I2C0)
SDA     →      Pin 18        I2C Data (I2C0)
VCC     →      5V            Power (for full 0-5V output range)
```

**Standard MCP4725 pinout (for reference):**
```
Pin 1 (VCC)  → 5V
Pin 2 (GND)  → GND
Pin 3 (SDA)  → Pin 18
Pin 4 (SCL)  → Pin 19
Pin 5 (A0)   → Address select (GND or VCC)
Pin 6 (OUT)  → Signal output
```

#### I2C Address Configuration (Critical for Separate Addressing)
MCP4725 has ONE address pin: **A0**

**How to set different addresses:**
- **MCP4725 #1 (0x60):** Connect A0 pin to **GND**
- **MCP4725 #2 (0x61):** Connect A0 pin to **VCC (5V)**

```
I2C Address    A0 Pin        Use Case
-----------    ------        --------
0x60           GND           DAC 1 - Pitch/CV Out
0x61           VCC (5V)      DAC 2 - Gate/Modulation Out
```

**Both DACs share the same SDA (18) and SCL (19) pins** - they are independent devices distinguished by address.

**Available Addresses** (if using 3+ DACs in future):
- 0x60 (A0=GND)
- 0x61 (A0=VCC)
- 0x62 (not on standard MCP4725, requires external address pins)
- 0x63 (not on standard MCP4725, requires external address pins)

> **Note:** Standard MCP4725 can only address 0x60 and 0x61. For more than 2 DACs, use MCP4725A with selectable A0/A1 pins, or use a different DAC like MCP4726 (4 addresses) or DAC8568 (8 channels, SPI).

#### TRRS Jack Wiring (4-Contact Stereo Jack)
```
TRRS Jack      Signal
---------      ------
Sleeve         GND
Ring 1         CV Out 1 (from MCP4725 #1)
Ring 2         CV Out 2 (from MCP4725 #2)
Tip            GND (or unused)
```

**Physical appearance:**
```
    Tip (top)      → GND
    |
    Ring 2 (mid)   → CV Out 2
    |
    Ring 1 (mid)   → CV Out 1
    |
    Sleeve (bottom)→ GND
```

#### Output Voltage Characteristics
- **Resolution:** 12-bit (4096 steps)
- **Output Range:** 0-5V DC
- **Settling Time:** <6 µs
- **Current:** ±50 mA
- **Accuracy:** ±0.5% of full scale

#### Firmware Usage (from config.h)
```cpp
#define MCP4725_ADDR_1  0x60   // CV Output 1 (Pitch)
#define MCP4725_ADDR_2  0x61   // CV Output 2 (Gate)
#define DAC_MAX_VALUE    4095  // 12-bit resolution
#define DAC_MAX_VOLTAGE  5.0f  // Full-scale output
```

#### Code Example: Setting DAC Voltages
```cpp
// In your firmware code:
#include <Adafruit_MCP4725.h>
#include <Wire.h>

Adafruit_MCP4725 dac1;  // Address 0x60
Adafruit_MCP4725 dac2;  // Address 0x61

void setup() {
    Wire.begin();  // I2C on pins 18 (SDA), 19 (SCL)
    
    // Initialize both DACs with different addresses
    if (!dac1.begin(MCP4725_I2CADDR_DEFAULT)) {  // 0x60
        Serial.println("DAC1 (0x60) not found!");
    }
    if (!dac2.begin(MCP4725_I2CADDR_DEFAULT + 1)) {  // 0x61
        Serial.println("DAC2 (0x61) not found!");
    }
}

void loop() {
    // Set CV1 to 2.5V (half of 5V)
    uint16_t value = 2048;  // 4095 * 0.5
    dac1.setVoltage(value, false);
    
    // Set CV2 to 5V (full voltage)
    dac2.setVoltage(4095, false);
}
```

#### Serial Monitor Output on Boot
```
Wire: I2C on pins 18 (SDA), 19 (SCL)
MCP4725 DAC1 initialized at address 0x60
MCP4725 DAC2 initialized at address 0x61
Polyphonion Ready!
```

---

### Old: BOOST-DAC8568 Board (Deprecated - SPI 8-Channel DAC)
**Status:** DISCONTINUED - Replaced with MCP4725 I2C solution
- Previous implementation used 8-channel 16-bit SPI DAC
- Issue: Hardware defect in BOOST-DAC8568 module (0V output)
- Current: Using 2x MCP4725 I2C DACs instead for reliability

---

## I2C Bus Configuration

The I2C bus (SDA=18, SCL=19) uses **Teensy 4.1's I2C0 interface** and is dedicated to DAC control:
- **MCP4725 DAC 1:** I2C Address 0x60 (A0=GND)
- **MCP4725 DAC 2:** I2C Address 0x61 (A0=VCC)

**No other I2C devices on this bus** - clean, dedicated connection.

**Note:** Teensy 4.1 also has I2C1 (pins 22/23) and I2C2 (pins 24/25) if future expansion needed.

## Power Requirements

- **Teensy 4.1:** USB power (5V) or external 5V via VIN
- **Display:** 3.3V from Teensy (draws ~30-50mA)
- **Touch Controller:** 3.3V from Teensy (minimal current)
- **Encoder:** 3.3V from Teensy (minimal current)
- **Buttons:** No power (uses pull-ups)
- **MCP4725 DACs:** 5V (critical for full 0-5V output range)

**Important:** MCP4725 VCC must be 5V for proper 0-5V output. Using 3.3V will limit output to 0-3.3V, insufficient for Eurorack standards (1V/octave requires headroom).

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

### MCP4725 DACs not detected
- **Verify I2C wiring:**
  - SDA (Pin 18) to both DAC SDA pins
  - SCL (Pin 19) to both DAC SCL pins
  - GND connections solid
- **Check A0 pin addressing:**
  - DAC1: A0 pin connected to GND → Address 0x60 ✓
  - DAC2: A0 pin connected to VCC (5V) → Address 0x61 ✓
- **Power check:**
  - Verify 5V to DAC VCC pins (not 3.3V)
  - GND properly connected
- **Serial output:** Should show "MCP4725 initialized at 0x60" and "MCP4725 initialized at 0x61"
- **Test with multimeter:**
  - Measure continuity on SDA/SCL lines
  - Check voltage at DAC OUT pins (should be 0-5V range)

### CV output voltage too low
- Check MCP4725 VCC is 5V (not 3.3V)
- Use multimeter to verify voltage at DAC OUT pins
- Verify TRRS jack ground connections (Sleeve = GND, Ring 1/2 = signals)
- Check firmware is addressing correct DAC address (0x60 or 0x61)

### No CV output on one DAC but not the other
- DAC that works: Addressing correct
- DAC that doesn't: Check A0 pin connection
  - If DAC2 silent: Verify A0 is connected to 5V (not floating)
  - If DAC1 silent: Verify A0 is connected to GND (not floating)
- Use I2C scanner to verify both addresses appear on bus

### CV output not changing when moving encoder
- Verify MCP4725 is receiving I2C commands (use serial debug output)
- Check which script is running and which DAC it's using
- Firmware mapping: LFO uses DAC1 (0x60), Sequencer gate uses DAC2 (0x61)

### I2C Bus Conflicts
- MCP4725 I2C bus (pins 18/19) is dedicated - no other I2C devices
- Teensy 4.1 has I2C1 (pins 22/23) and I2C2 (pins 24/25) available for future expansion
- Pull-up resistors on I2C bus already present on MCP4725 breakout boards (usually 10kΩ)

## Serial Monitor Output

At startup, you should see:
```
Polyphonion Initializing...
Wire: I2C on pins 18 (SDA), 19 (SCL)
MCP4725 DAC1 initialized at address 0x60
MCP4725 DAC2 initialized at address 0x61
LFO: Initialized successfully with MCP4725 DAC
     CV output available on TRRS jack (CV Out 1)
SEQ: CV DAC initialized (address 0x60)
     Gate DAC initialized (address 0x61)
ScriptManager: Initialized
Polyphonion Ready!
```

**If DACs are not detected:**
- Check I2C wiring (pins 18/19)
- Verify A0 addressing pins
- Monitor serial output for specific error messages
- Try I2C address scanner to verify both 0x60 and 0x61 respond
