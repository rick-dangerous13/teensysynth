# MCP4725 Setup Guide - Complete Wiring & Configuration

## Overview
You need to wire **2x MCP4725 DACs** to your Teensy 4.1 to output CV signals via TRRS jacks. The key to separate addressing is the **A0 pin** on each module.

---

## Step 1: Physical Wiring

### What You Need
- 2x MCP4725 modules (or breakout boards)
- Teensy 4.1
- 2x TRRS (Tip-Ring-Ring-Sleeve) jacks
- I2C wires: SDA (Pin 18) and SCL (Pin 19)
- 5V power supply (for full 0-5V range)
- GND connections
- A0 address wires (one to GND, one to 5V)

**Note:** MCP4725 breakout boards vary in pin labeling. Common labels are:
- `OUT, GND, SCL, SDA, VCC` (your board)
- `VCC, GND, SDA, SCL, A0, OUT` (standard)
- Check your specific board documentation

### Wiring Diagram

```
┌─────────────────────────────────────────────────────────────┐
│                     TEENSY 4.1                              │
│                                                             │
│  Pin 18 (SDA) ─────────────┬─────────────────────────────── │
│                            │                               │
│  Pin 19 (SCL) ─────────────┼─────────────────────────────── │
│                            │                               │
│  5V (Power) ───────────────┼────────┐                      │
│                            │        │                      │
│  GND ──────────────────────┼────────┼────────────┐          │
│                            │        │            │          │
│                            ▼        ▼            ▼          │
│                    ┌──────────────┬──────────────┐          │
│                    │ I2C Bus      │ Power/Ground │          │
│                    │ SDA/SCL      │              │          │
│                    └──┬────────┬──┘              │          │
│                       │        │                │          │
│                       │        │                │          │
│   ┌────────────────┐  │        │      ┌────────┴─┐         │
│   │ MCP4725 #1     │◄─┘        │      │           │         │
│   │ 0x60           │           │      │ (VCC)    │         │
│   │ OUT ──────────────────► TRRS Jack 1  CV1   │         │
│   │ A0 ──►GND      │           │      │     (GND)│         │
│   └────────────────┘           │      └────────┬─┘         │
│                                │               │            │
│   ┌────────────────┐           │      ┌────────┴─┐         │
│   │ MCP4725 #2     │◄──────────┘      │           │         │
│   │ 0x61           │                  │  (5V)    │         │
│   │ OUT ──────────────────► TRRS Jack 2  CV2   │         │
│   │ A0 ──►5V (VCC) │                  │     (GND)│         │
│   └────────────────┘                  └─────────┘         │
└─────────────────────────────────────────────────────────────┘
```

### Connection Checklist

**For MCP4725 #1 (will be address 0x60):**
- [ ] VCC → 5V (external power or Teensy 5V)
- [ ] GND → GND (common ground)
- [ ] SDA → Pin 18
- [ ] SCL → Pin 19
- [ ] **A0 → GND** (This makes it address 0x60)
- [ ] OUT → TRRS Jack 1 (Ring 1)

**For MCP4725 #2 (will be address 0x61):**
- [ ] VCC → 5V (same power rail)
- [ ] GND → GND (same ground)
- [ ] SDA → Pin 18 (same bus!)
- [ ] SCL → Pin 19 (same bus!)
- [ ] **A0 → VCC (5V)** (This makes it address 0x61)
- [ ] OUT → TRRS Jack 2 (Ring 2)

**TRRS Jacks:**
- [ ] Sleeve (bottom) → GND
- [ ] Ring 1 → CV Out 1 (from DAC #1)
- [ ] Ring 2 → CV Out 2 (from DAC #2)
- [ ] Tip (top) → GND (or unused)

---

## Step 2: Understanding I2C Address Configuration

### The Critical Part: A0 Pin

The MCP4725 has **ONE address pin (A0)** that determines which I2C address it responds to:

| DAC | A0 Connection | I2C Address | Firmware Address |
|-----|---------------|-------------|------------------|
| #1  | Connected to GND | **0x60** | `MCP4725_ADDR_1 = 0x60` |
| #2  | Connected to VCC (5V) | **0x61** | `MCP4725_ADDR_2 = 0x61` |

### How It Works

The I2C protocol sends commands with an **address byte** that identifies which device should listen:

```
Teensy (Master)              I2C Bus                     DACs (Slaves)
                                                         
send("Hello 0x60", data)  ──► "0x60: set voltage=2500" ──► DAC #1 responds
                                                              (A0=GND)
send("Hello 0x61", data)  ──► "0x61: set voltage=5000" ──► DAC #2 responds
                                                              (A0=VCC)
```

Both DACs listen on the same **SDA (18) and SCL (19)** wires, but only respond to their specific address.

### Critical: A0 Pin Must Be Connected

- **Do NOT leave A0 floating (unconnected)** — this causes unpredictable addressing
- **Do NOT connect A0 to SDA or SCL** — only connect to GND or VCC
- **Verify visually:** Check that A0 wire goes to either GND or 5V, nothing else

---

## Step 3: Firmware Configuration

Your `config.h` already has the correct settings:

```cpp
// From include/config.h
#define MCP4725_ADDR_1  0x60   // First MCP4725 (A0=GND)
#define MCP4725_ADDR_2  0x61   // Second MCP4725 (A0=VCC)
#define DAC_MAX_VALUE    4095  // 12-bit resolution
```

The firmware automatically initializes both DACs at different addresses during startup.

---

## Step 4: Verify Everything Works

### Serial Monitor Output (115200 baud)

After uploading, open the serial monitor and look for:

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

### If You See Errors

**"MCP4725 DAC1 not found"** or **"MCP4725 DAC2 not found":**
1. Check I2C wiring (pins 18/19 connected to both DACs)
2. Verify A0 pin on the missing DAC:
   - If DAC1 not found: Ensure A0 is connected to GND
   - If DAC2 not found: Ensure A0 is connected to VCC
3. Use I2C address scanner to verify bus connectivity
4. Check 5V power to DACs (multimeter test)

---

## Step 5: Test CV Output

### With Multimeter
1. Set multimeter to **20V DC** range
2. Black probe → TRRS Sleeve (GND)
3. Red probe → TRRS Ring 1 (CV Out 1)
4. Rotate encoder or use menu to adjust parameters
5. Voltage should sweep from **0V to 5V** as you change values

### Expected Behavior
- **DAC #1 (0x60):** Outputs LFO pitch CV (0-5V as you adjust frequency/level)
- **DAC #2 (0x61):** Outputs gate/modulation signal

---

## Troubleshooting Flowchart

```
DACs not detected on startup?
├─ Is 5V power connected to both DACs? (measure with multimeter)
│  └─ No → Connect 5V supply
│
├─ Are SDA/SCL wired to pins 18/19? (check both DACs)
│  └─ No → Rewire
│
├─ Is A0 pin connected on both DACs?
│  ├─ DAC #1: A0 connected to GND? 
│  │  └─ No → Connect to GND
│  └─ DAC #2: A0 connected to VCC?
│     └─ No → Connect to 5V
│
└─ Run I2C address scanner to verify both 0x60 and 0x61 appear

No CV output despite serial showing "initialized"?
├─ Are TRRS jacks wired correctly?
│  └─ Sleeve=GND, Ring1=CV1, Ring2=CV2?
│
├─ Is firmware actually sending commands to the DACs?
│  └─ Check serial debug output or add debug prints
│
└─ Test with multimeter (0-5V range, black to GND, red to Ring pins)
```

---

## Quick Reference: Pin Summary

### Teensy 4.1 I2C Connections
| Pin | Signal | Connects To |
|-----|--------|-------------|
| 18 | SDA | Both MCP4725 SDA pins |
| 19 | SCL | Both MCP4725 SCL pins |
| 5V | Power | Both MCP4725 VCC + MCP4725 #2 A0 |
| GND | Ground | Both MCP4725 GND + MCP4725 #1 A0 |

### MCP4725 Unique Connections
| DAC | A0 Pin | Address |
|-----|--------|---------|
| #1 | Connected to GND | 0x60 |
| #2 | Connected to VCC | 0x61 |

---

## Future Expansion

If you need **more than 2 DACs**, options include:

1. **MCP4725A** - Has additional address pins (can support 0x62, 0x63)
2. **DAC8568** - 8-channel SPI DAC (your original choice, but hardware was defective)
3. **PCF8574** - I2C expander + external DACs
4. Use Teensy's **second I2C bus (pins 22/23)** with different DAC modules

---

## References

- **I2C Protocol:** Each device has a unique address; firmware selects device by address byte
- **MCP4725 Datasheet:** Check A0 pin configuration in section 5.1
- **Teensy I2C:** Pins 18/19 are I2C0 (primary); pins 22/23 are I2C1 (secondary)
- **12-bit Resolution:** 4096 steps = (5V / 4096) = 1.22 mV per step

---

## Success Indicators

✓ Serial monitor shows both DACs initialized at 0x60 and 0x61  
✓ Rotating encoder changes displayed CV value in UI  
✓ Multimeter shows 0-5V sweep on TRRS jacks when parameters change  
✓ No I2C error messages in serial output  
✓ TRRS jacks output smooth CV control voltage  

You're done! 🎵
