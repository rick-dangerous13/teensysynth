# Dual MCP4725 DAC Setup Using TCA9548A I2C Multiplexer

## Overview

Since your MCP4725 boards don't have individual address pins (A0/A1/A2), we use a **TCA9548A I2C multiplexer** to control two MCP4725 DACs independently. The multiplexer switches between different I2C channels, allowing each DAC to appear as if it's at a different address.

```
Teensy 4.1
    |
    +--I2C (Pin 18=SDA, Pin 19=SCL)
    |
    v
TCA9548A Multiplexer (0x70)
    |
    +--Channel 0 --> MCP4725 #1 (0x60) = Pitch CV
    |
    +--Channel 1 --> MCP4725 #2 (0x60) = Gate CV
```

## Hardware Specifications

### TCA9548A Multiplexer
- **Chip**: Texas Instruments TCA9548A
- **I2C Address**: 0x70 (when A0=GND, A1=GND, A2=GND)
- **Channels**: 8 (we use 0 and 1)
- **Current**: ~10mA typical
- **Supply**: 2.3V to 5.5V

### MCP4725 DACs
- **Chip**: Microchip MCP4725
- **I2C Address**: 0x60 (both can be 0x60 since they're on different multiplexer channels)
- **Resolution**: 12-bit
- **Output**: 0V to 5V

## Wiring Diagram

### Teensy 4.1 to TCA9548A Multiplexer

| Teensy Pin | Signal | TCA9548A Pin | Purpose |
|-----------|--------|--------------|---------|
| 5V        | VCC    | VIN          | Power   |
| GND       | GND    | GND          | Ground  |
| 18        | SDA    | SDA          | I2C Data |
| 19        | SCL    | SCL          | I2C Clock |
| 5V        | -      | RST          | Reset (pulled high) |
| GND       | -      | A0           | Address bit 0 |
| GND       | -      | A1           | Address bit 1 |
| GND       | -      | A2           | Address bit 2 |

**Result**: Multiplexer address = 0x70

### TCA9548A Channel 0 to MCP4725 #1 (Pitch CV)

| TCA9548A Pin | Signal | MCP4725 #1 Pin | Purpose |
|-------------|--------|----------------|---------|
| SD0         | SDA    | SDA            | I2C Data |
| SC0         | SCL    | SCL            | I2C Clock |

### TCA9548A Channel 1 to MCP4725 #2 (Gate CV)

| TCA9548A Pin | Signal | MCP4725 #2 Pin | Purpose |
|-------------|--------|----------------|---------|
| SD1         | SDA    | SDA            | I2C Data |
| SC1         | SCL    | SCL            | I2C Clock |

### Both MCP4725 DACs (Power and Output)

| MCP4725 Pin | Connection | Purpose |
|------------|-----------|---------|
| VCC        | Teensy 5V | Power   |
| GND        | Teensy GND | Ground  |
| OUT        | Synth Module CV Input | Audio/CV Output |

## Complete Wiring Summary

```
TEENSY 4.1
  5V ──────────┬──────────────────────────────────┬─────────┬─────────┐
               │                                  │         │         │
            TCA9548A                          MCP4725#1  MCP4725#2  Other
            VIN (1)                          VCC        VCC      devices
               │
  GND ─────────┼──────────────────────────────────┼─────────┼─────────┐
               │                                  │         │         │
            TCA9548A            MCP4725#1       MCP4725#2            GND
            GND (2)             GND             GND
            A0 (3)
            A1 (4)
            A2 (5)
               │
  Pin 18 ──────┼──────────┐
  (SDA)        │          │
            TCA9548A      Each MCP4725 #1
            SDA (6)       SDA (via CH0 SD0)
               │
               ├──────────┐
               │          │
            SD0 (15)      MCP4725 #1
                          SDA
               │
               ├──────────┐
               │          │
            SD1 (16)      MCP4725 #2
                          SDA
               │
  Pin 19 ──────┼──────────┐
  (SCL)        │          │
            TCA9548A      Each MCP4725
            SCL (7)       SCL (via CH0/CH1 SC0/SC1)
               │
               ├──────────┐
               │          │
            SC0 (17)      MCP4725 #1
                          SCL
               │
               ├──────────┐
               │          │
            SC1 (18)      MCP4725 #2
                          SCL
               │
            
  RST ─────────┘
           (pulled to 5V)
```

## Step-by-Step Wiring Instructions

### 1. Power and Ground (Critical - do this first)
```
Teensy 5V  → TCA9548A VIN
Teensy GND → TCA9548A GND
Teensy GND → TCA9548A A0
Teensy GND → TCA9548A A1
Teensy GND → TCA9548A A2
Teensy 5V  → TCA9548A RST

Teensy 5V  → MCP4725 #1 VCC
Teensy GND → MCP4725 #1 GND
Teensy 5V  → MCP4725 #2 VCC
Teensy GND → MCP4725 #2 GND
```

### 2. I2C Bus (Main communication)
```
Teensy Pin 18 (SDA) → TCA9548A SDA
Teensy Pin 19 (SCL) → TCA9548A SCL
```

### 3. Multiplexer Channel 0 to MCP4725 #1 (Pitch CV)
```
TCA9548A SD0 → MCP4725 #1 SDA
TCA9548A SC0 → MCP4725 #1 SCL
```

### 4. Multiplexer Channel 1 to MCP4725 #2 (Gate CV)
```
TCA9548A SD1 → MCP4725 #2 SDA
TCA9548A SC1 → MCP4725 #2 SCL
```

### 5. Output Connections
```
MCP4725 #1 OUT → Synth Pitch CV Input
MCP4725 #2 OUT → Synth Gate CV Input
```

## Pull-up Resistors

The TCA9548A and MCP4725 chips need I2C pull-up resistors. You should have:

- **4.7kΩ on SDA (Teensy Pin 18)** from Teensy to 5V
- **4.7kΩ on SCL (Teensy Pin 19)** from Teensy to 5V

**Note**: If you already have pull-ups on your Teensy board or breakout boards, you don't need additional ones. Check first to avoid over-pulling.

## Configuration in Code

Update `include/config.h`:

```cpp
// TCA9548A Multiplexer address (A0=GND, A1=GND, A2=GND = 0x70)
#define TCA9548A_ADDR         0x70

// MCP4725 addresses (both use 0x60 since they're on different channels)
#define MCP4725_ADDR_1        0x60  // Pitch CV (on multiplexer channel 0)
#define MCP4725_ADDR_2        0x60  // Gate CV (on multiplexer channel 1)

// Multiplexer channel assignments
#define MCP4725_CHANNEL_1     0     // Channel 0 for Pitch CV
#define MCP4725_CHANNEL_2     1     // Channel 1 for Gate CV
```

## How It Works

1. **Teensy initializes I2C bus** to Teensy pins 18 (SDA) and 19 (SCL)
2. **Teensy sends command to TCA9548A** to enable Channel 0
3. **Teensy communicates with MCP4725 #1** on address 0x60 (via Channel 0)
4. **Teensy sends command to TCA9548A** to enable Channel 1
5. **Teensy communicates with MCP4725 #2** on address 0x60 (via Channel 1)

The multiplexer automatically switches which channel is active, so both DACs can use the same I2C address without conflict.

## Troubleshooting

### All Devices Not Found
```
✗ TCA9548A (0x70): NOT FOUND
✗ DAC1: NOT FOUND
✗ DAC2: NOT FOUND
```

**Check:**
1. Power (5V and GND) on TCA9548A
2. SDA (pin 18) and SCL (pin 19) wired to TCA9548A
3. Pull-up resistors (4.7kΩ) on SDA and SCL
4. A0, A1, A2 pins on TCA9548A connected to GND

### TCA9548A Found But DACs Not Found
```
✓ TCA9548A (0x70): OK
✗ DAC1: NOT FOUND
✗ DAC2: NOT FOUND
```

**Check:**
1. Channel 0 wiring: SD0 → MCP4725 #1 SDA, SC0 → MCP4725 #1 SCL
2. Channel 1 wiring: SD1 → MCP4725 #2 SDA, SC1 → MCP4725 #2 SCL
3. Power to both MCP4725 chips (VCC and GND)
4. No loose wires

### One DAC Found But Not the Other
```
✓ TCA9548A (0x70): OK
✓ DAC1: OK
✗ DAC2: NOT FOUND
```

**Check:**
1. Channel 1 specifically: SD1 and SC1 wiring
2. Power and ground on MCP4725 #2
3. No crossed wires between channels

## I2C Address Reference

| Device | Address | Pins |
|--------|---------|------|
| TCA9548A (multiplexer) | 0x70 | A0=GND, A1=GND, A2=GND |
| MCP4725 #1 (via CH0) | 0x60 | On multiplexer channel 0 |
| MCP4725 #2 (via CH1) | 0x60 | On multiplexer channel 1 |

## Pin Labels Reference

Your TCA9548A board pins in order:

```
Pin  1: VIN
Pin  2: GND
Pin  3: A0
Pin  4: A1
Pin  5: A2
Pin  6: SDA (main I2C)
Pin  7: SCL (main I2C)
Pin  8: GND (another ground)
Pin  9: NC (not connected)
Pin 10: NC
Pin 11: NC
Pin 12: NC
Pin 13: SC7 (Channel 7 SCL)
Pin 14: SD7 (Channel 7 SDA)
Pin 15: SD0 (Channel 0 SDA) ← Use this for MCP4725 #1
Pin 16: SC0 (Channel 0 SCL) ← Use this for MCP4725 #1
Pin 17: SC1 (Channel 1 SCL) ← Use this for MCP4725 #2
Pin 18: SD1 (Channel 1 SDA) ← Use this for MCP4725 #2
Pin 19: SC2 (Channel 2 SCL)
Pin 20: SD2 (Channel 2 SDA)
... (and so on for channels 3-6)
```

---

**Next Steps:**
1. Wire everything according to the pinout above
2. Update `include/config.h` with multiplexer settings
3. Rebuild and upload firmware
4. Monitor serial output at 115200 baud to verify all devices are detected
