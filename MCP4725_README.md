# MCP4725 Documentation - Complete Package

## You Asked:
> "How do I wire up the MCPs and how do you make sure you address them separately?"

## The Answer (TL;DR):

**Wire A0 pin differently on each DAC:**
- DAC #1: A0 → GND (becomes address 0x60)
- DAC #2: A0 → VCC (becomes address 0x61)

Both DACs share the same I2C bus (pins 18 & 19), but respond to different addresses. I2C protocol routes commands automatically to the correct DAC based on address.

---

## Documentation Files Created

### 1. **MCP4725_QUICK_START.md** (⭐ Start Here!)
- **What:** One-page printable reference card
- **Best for:** Wiring while building
- **Length:** 2 minutes to read
- **Contains:** Wiring diagram, checklist, quick troubleshooting

### 2. **MCP4725_SETUP_GUIDE.md** (Complete Guide)
- **What:** Step-by-step wiring and configuration
- **Best for:** First-time implementation
- **Length:** 15 minutes to read
- **Contains:** 
  - Detailed wiring instructions
  - I2C addressing explanation
  - Firmware code examples
  - Verification procedures
  - Full troubleshooting flowchart

### 3. **MCP4725_ADDRESS_GUIDE.md** (Deep Dive)
- **What:** Technical explanation of I2C addressing
- **Best for:** Understanding how it works
- **Length:** 20 minutes to read
- **Contains:**
  - I2C protocol basics
  - Address byte breakdown
  - Communication flow diagrams
  - Common mistakes analysis
  - Why the A0 pin works at hardware level

### 4. **HARDWARE_WIRING.md** (Updated Reference)
- **What:** Full hardware reference for entire system
- **Best for:** Complete system setup and troubleshooting
- **Changes:** 
  - Added MCP4725 quick reference table
  - Replaced DAC8568 section with MCP4725 details
  - Updated I2C bus configuration
  - Added MCP4725-specific troubleshooting

### 5. **README.md** (Updated)
- **What:** Project overview with documentation links
- **Changes:**
  - Added MCP4725 to hardware requirements
  - Added links to all documentation

---

## The Technical Summary

### Problem
You have two MCP4725 DACs that need to be independently addressed on a shared I2C bus (only 2 wires: SDA and SCL).

### Your MCP4725 Modules
Your boards have these pin labels: **OUT, GND, SCL, SDA, VCC**

The **A0 pin may not be labeled** - it's often a tiny solder pad on the back or near the IC chip. See MCP4725_PINOUT_REFERENCE.md for how to find it!

### Solution
The MCP4725 has one programmable address pin (**A0**) that determines the I2C address:

| Configuration | A0 Connection | I2C Address |
|---------------|---------------|-------------|
| DAC #1 | Connected to GND | 0x60 |
| DAC #2 | Connected to VCC | 0x61 |

### How It Works
1. **I2C Address Byte** contains both the device address (0x60 or 0x61) and a read/write bit
2. When Teensy sends a command, it includes the address byte
3. Each DAC only responds to its configured address
4. The A0 pin wiring determines which address the DAC responds to
5. Both DACs listen on the same 2 wires (SDA=18, SCL=19), but only the addressed one responds

### Why This Works
- **Hardware-level addressing**: The MCP4725 chip internally decodes the A0 pin and stores that as its address
- **I2C protocol handles routing**: The protocol automatically selects the correct device based on address
- **No chip select pins needed**: Unlike SPI, I2C uses address bytes instead of separate select pins

---

## Quick Wiring Reference

```
MCP4725 #1                MCP4725 #2
├─ VCC → 5V              ├─ VCC → 5V
├─ GND → GND             ├─ GND → GND
├─ SDA → Pin 18          ├─ SDA → Pin 18  ← SAME PIN!
├─ SCL → Pin 19          ├─ SCL → Pin 19  ← SAME PIN!
├─ A0 → GND              ├─ A0 → VCC (5V)
└─ OUT → TRRS Ring 1     └─ OUT → TRRS Ring 2

Result:
├─ DAC #1 responds to address 0x60
├─ DAC #2 responds to address 0x61
└─ Firmware can control them independently!
```

---

## Critical Reminders

❌ **DO NOT:**
- Leave A0 pin floating (unconnected) — causes unpredictable addressing
- Connect A0 to SDA or SCL — corrupts I2C communication
- Wire both A0 pins to GND — both DACs respond to 0x60
- Wire both A0 pins to VCC — both DACs respond to 0x61

✓ **DO:**
- Connect A0 on DAC #1 explicitly to GND
- Connect A0 on DAC #2 explicitly to VCC (5V)
- Use same SDA (18) and SCL (19) pins for both DACs (shared I2C bus)
- Give both DACs 5V power and common ground
- Verify with serial monitor: should show both 0x60 and 0x61 detected

---

## Verification

After uploading firmware, open serial monitor at 115200 baud:

**Expected Output (Success):**
```
MCP4725 DAC1 initialized at address 0x60
MCP4725 DAC2 initialized at address 0x61
```

**If You See Error:**
- One DAC not found → Check that DAC's A0 wiring
- Both not found → Check I2C wiring (pins 18/19)
- Intermittent detection → Check that A0 pins aren't floating

---

## Architecture Diagram

```
I2C Bus (2 wires)
    │
    ├─ SDA (Pin 18) ──┬──► MCP4725 #1 (A0=GND, address 0x60)
    │                 └──► MCP4725 #2 (A0=VCC, address 0x61)
    │
    └─ SCL (Pin 19) ──┬──► MCP4725 #1
                      └──► MCP4725 #2

Teensy firmware:
    dac1.setVoltage()  ──► [I2C: 0x60 address byte] ──► DAC #1 responds
    dac2.setVoltage()  ──► [I2C: 0x61 address byte] ──► DAC #2 responds
```

---

## Why Standard I2C Addressing Works

I2C is designed for multiple devices on shared bus using addresses:
- **No chip select pins needed** (unlike SPI)
- **Address byte sent first** to select device
- **Each device decodes address** and responds only to its own
- **Perfect for our two-DAC scenario**

The MCP4725's A0 pin is the **hardware address selector** — it determines what address the chip internally recognizes.

---

## Next Steps

1. **Print MCP4725_QUICK_START.md** and keep it handy while wiring
2. **Follow the wiring** from MCP4725_SETUP_GUIDE.md
3. **Upload firmware** (already configured in config.h)
4. **Check serial monitor** for both DACs detected
5. **Use TRRS jacks** for CV output

Once working:
- Rotate encoder → CV output changes on jacks
- Test with multimeter → 0-5V sweep
- Connect to Eurorack → Control synth with CV!

---

## Questions?

Refer to the appropriate documentation:
- **"How do I wire it?"** → MCP4725_SETUP_GUIDE.md
- **"Why does A0 work this way?"** → MCP4725_ADDRESS_GUIDE.md  
- **"How do I troubleshoot?"** → HARDWARE_WIRING.md (Troubleshooting section)
- **"Just give me the essentials"** → MCP4725_QUICK_START.md

---

**Status: Ready to wire! 🎵**

All documentation is in the repo. You have everything needed to:
1. Wire the MCPs correctly
2. Understand why the A0 pin creates separate addresses
3. Verify everything works
4. Troubleshoot if needed
