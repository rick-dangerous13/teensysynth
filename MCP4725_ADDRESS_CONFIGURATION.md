# MCP4725 I2C Address Configuration (No Soldering Required)

## Overview
The MCP4725 single-channel DAC uses a 3-bit address selection through the A0, A1, and A2 pins. You can configure two different MCP4725 chips on the same I2C bus by setting these pins to different levels using **jumper wires** - no soldering required.

## I2C Address Selection Table

| A2 Pin | A1 Pin | A0 Pin | I2C Address | Hex  |
|--------|--------|--------|-------------|------|
| GND    | GND    | GND    | 0x60        | 96   |
| GND    | GND    | VCC    | 0x61        | 97   |
| GND    | VCC    | GND    | 0x62        | 98   |
| GND    | VCC    | VCC    | 0x63        | 99   |
| VCC    | GND    | GND    | 0x64        | 100  |
| VCC    | GND    | VCC    | 0x65        | 101  |
| VCC    | VCC    | GND    | 0x66        | 102  |
| VCC    | VCC    | VCC    | 0x67        | 103  |

## Recommended Configuration for Polyphonion

**DAC 1 (Pitch CV):**
```
A2 -> GND
A1 -> GND
A0 -> GND
Address: 0x60
```

**DAC 2 (Gate CV):**
```
A2 -> GND
A1 -> GND
A0 -> VCC
Address: 0x61
```

This is the simplest configuration and is the default in the code.

## How to Configure Without Soldering

### Materials Needed
- Two MCP4725 breakout boards
- Jumper wires (male-to-male or male-to-female, depending on your board)
- Teensy 4.1
- Small breadboard (optional, for organizing jumpers)

### Step-by-Step Instructions

#### 1. Connect I2C Bus (Shared between both MCPs)
```
MCP4725 #1 SDA -> Teensy Pin 18 (I2C SDA)
MCP4725 #1 SCL -> Teensy Pin 19 (I2C SCL)
MCP4725 #2 SDA -> Teensy Pin 18 (I2C SDA) [same as #1]
MCP4725 #2 SCL -> Teensy Pin 19 (I2C SCL) [same as #1]
```

#### 2. Connect Power (Both MCPs)
```
MCP4725 #1 VCC -> Teensy 5V
MCP4725 #1 GND -> Teensy GND
MCP4725 #2 VCC -> Teensy 5V
MCP4725 #2 GND -> Teensy GND
```

#### 3. Configure Address Pins for MCP4725 #1 (Pitch CV, Address 0x60)
```
A0 -> GND (jumper to GND rail)
A1 -> GND (jumper to GND rail)
A2 -> GND (jumper to GND rail) [optional, can leave floating]
```

#### 4. Configure Address Pins for MCP4725 #2 (Gate CV, Address 0x61)
```
A0 -> VCC (jumper to 5V rail)
A1 -> GND (jumper to GND rail)
A2 -> GND (jumper to GND rail) [optional, can leave floating]
```

#### 5. Configure Output Pins
```
DAC1 OUT (MCP #1) -> Pitch CV output (to your synth module)
DAC2 OUT (MCP #2) -> Gate CV output (to your synth module)
```

## Updating the Configuration

If you use different addresses, update `include/config.h`:

```cpp
#define MCP4725_ADDR_1  0x60   // Change this to your Pitch CV DAC address
#define MCP4725_ADDR_2  0x61   // Change this to your Gate CV DAC address
```

Then rebuild and upload the firmware.

## Troubleshooting

### DACs Not Detected
If you see:
```
✗ DAC1 (0x60): NOT FOUND
✗ DAC2 (0x61): NOT FOUND
```

Check:
1. **I2C Wiring**: Verify SDA (pin 18) and SCL (pin 19) are correctly wired to both MCPs
2. **Address Jumpers**: Confirm A0/A1/A2 pins are properly connected to GND or 5V
3. **Pull-up Resistors**: Add 4.7kΩ pull-ups on SDA and SCL if not present
4. **Power**: Verify both MCPs have VCC at 5V

### One DAC Detected, One Not
Example output:
```
✓ DAC1 (0x60): OK
✗ DAC2 (0x61): NOT FOUND
```

This usually means:
- The address jumpers on DAC2 are not correct
- DAC2 has a loose wire
- DAC2 address is set to something other than 0x61

Double-check the A0 pin on the second MCP4725 is connected to VCC (5V).

### Garbled CV Output or Gate Not Responding
If CV output seems erratic or gate doesn't trigger:
1. Check I2C clock speed is set to 100kHz (the code sets this automatically)
2. Verify SDA and SCL pull-ups are 4.7kΩ
3. Ensure no wire is loose or touching another connection

## Example Breadboard Layout

```
        VCC(5V)              GND
          |                   |
    +-----+-----+         +---+---+
    |           |         |       |
   [MCP1]      [MCP2]    |       |
    A0->+----[Jumper]----+       |  (A0=GND for MCP1)
    A1->+------------------+     |  (A1=GND for both)
    A2->+------------------+--+--+  (A2=GND for both)

   [MCP1] A0->VCC[Jumper to 5V]  (A0=VCC for MCP2)
   
   Both SDA and SCL share:
   SDA->Teensy 18
   SCL->Teensy 19
```

## I2C Protocol Details (For Reference)

The MCP4725 uses a simple I2C protocol:
- **Clock Speed**: 100 kHz (standard mode) or 400 kHz (fast mode) - code uses 100 kHz for stability
- **Address Bits**: 7-bit addressing (A2 A1 A0 followed by R/W bit)
- **Data Format**: 12-bit DAC value sent as two bytes

When you call `dac1.setVoltage(dacValue, false)`:
1. The library sends the address byte (0x60 for DAC1, 0x61 for DAC2) with write bit
2. Followed by the 12-bit DAC value
3. The MCP4725 immediately updates its output

## Power Supply Note

For full 0-5V output range and stable operation:
- MCP4725 VCC should be 5V (not 3.3V)
- Use a separate 5V supply if possible
- Add a 100µF capacitor near MCP4725 VCC pin for stability

---

**Questions?** Check `config.h` for the current address definitions, or review the poliquencer initialization logs via Serial Monitor at 115200 baud.
