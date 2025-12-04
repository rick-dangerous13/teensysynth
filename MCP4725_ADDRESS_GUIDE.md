# MCP4725 I2C Address Configuration - Visual Guide

## The Core Concept: How Two DACs Share One I2C Bus

```
SINGLE I2C BUS (2 wires: SDA & SCL)
│
├─ Both MCP4725 modules listen on same 2 wires
├─ Firmware sends address byte first: "Hey 0x60!" or "Hey 0x61!"
└─ Each DAC only responds to its configured address
```

---

## The A0 Pin: Your Address Selector

The MCP4725 has ONE programmable address pin: **A0**

### How It Works (Hardware Level)

```
┌──────────────────────────────┐
│   MCP4725 Chip               │
├──────────────────────────────┤
│                              │
│  I2C Address = 0x60 or 0x61  │ ◄─ Determined by A0 pin
│  ↑                           │
│  └─ Hard-wired at chip level │
│                              │
│  If A0 = GND   → 0x60        │
│  If A0 = VCC   → 0x61        │
│                              │
└──────────────────────────────┘
```

### Wiring A0 (The Critical Part)

```
MCP4725 #1 (becomes 0x60)        MCP4725 #2 (becomes 0x61)
───────────────────────           ───────────────────────

     VCC ──→ 5V                         VCC ──→ 5V
     GND ──→ GND                        GND ──→ GND
     SDA ──→ Pin 18                     SDA ──→ Pin 18
     SCL ──→ Pin 19                     SCL ──→ Pin 19
     A0  ──→ GND ✓                      A0  ──→ VCC (5V) ✓
     OUT ──→ TRRS Jack 1                OUT ──→ TRRS Jack 2
```

---

## I2C Communication Example

Here's what happens in the firmware:

```cpp
// In your firmware (simplified)

// Initialize I2C bus (shared by both DACs)
Wire.begin();  // SDA=18, SCL=19

// Create two DAC objects with different addresses
Adafruit_MCP4725 dac1;  // Will use 0x60
Adafruit_MCP4725 dac2;  // Will use 0x61

// During initialization:
dac1.begin(0x60);  // ← This address matches A0=GND
dac2.begin(0x61);  // ← This address matches A0=VCC

// Later, in your main loop:
dac1.setVoltage(2048);  // Sends: [0x60 address byte] [data bytes]
                        // DAC #1 listens, responds ✓
                        // DAC #2 ignores (address doesn't match)

dac2.setVoltage(4095);  // Sends: [0x61 address byte] [data bytes]
                        // DAC #1 ignores (address doesn't match)
                        // DAC #2 listens, responds ✓
```

---

## The I2C Bus: What Happens on the Wire

```
Teensy 4.1                          I2C Bus (SDA 18, SCL 19)              Both MCP4725s
Master                              ──────────────────                     Slaves

dac1.setVoltage(2048)
        │
        ├─ Check: Is this address 0x60?
        │         (matches my A0 pin = GND)
        ├─ Send I2C start condition ────────► SDA goes LOW
        │                                    SCL stays HIGH
        │
        ├─ Send address byte 0x60  ────────► "01100000" on wire
        │   + Write bit (0)
        │
        ├─ Wait for ACK ◄──────────────────── DAC #1 pulls SDA LOW
        │   (DAC #1 says "yes, that's me!")   DAC #2 stays quiet
        │
        ├─ Send data bytes (2048 = 0x800) ─► Data flows
        │
        └─ Send I2C stop condition ────────► SDA goes HIGH
                                             (transaction complete)


dac2.setVoltage(4095)
        │
        ├─ Check: Is this address 0x61?
        │         (matches my A0 pin = VCC)
        ├─ Send I2C start condition ────────► SDA goes LOW
        │                                    SCL stays HIGH
        │
        ├─ Send address byte 0x61  ────────► "01100001" on wire
        │   + Write bit (0)
        │
        ├─ Wait for ACK ◄──────────────────── DAC #2 pulls SDA LOW
        │   (DAC #2 says "yes, that's me!")   DAC #1 stays quiet
        │
        ├─ Send data bytes (4095 = 0xFFF) ─► Data flows
        │
        └─ Send I2C stop condition ────────► SDA goes HIGH
                                             (transaction complete)
```

---

## Address Byte Breakdown

In binary, the I2C address byte is:

```
7-bit Address               R/W
     │                      │
┌────┴────┐                │
01100000  0  ◄─ 0x60 (read=1, write=0)
01100000  1  ◄─ 0x61 (read=1, write=0)

Where:
- 0110000_ = MCP4725 base address (fixed)
- Last bit (A0) = Depends on your wiring!
  - 0 (GND)  → 0x60
  - 1 (VCC)  → 0x61
```

---

## Common Mistakes & Fixes

### ❌ Mistake 1: A0 Floating (Not Connected)

```
MCP4725 A0 pin ──► (nothing connected)

Problem:
- A0 reads as random (1 or 0)
- DAC responds to random address
- Firmware can't find DAC consistently
- Serial shows: "MCP4725 not found" (intermittent)

Fix: Connect A0 to either GND or VCC explicitly
```

### ❌ Mistake 2: Both A0 Connected to GND

```
MCP4725 #1: A0 ──► GND ✓
MCP4725 #2: A0 ──► GND ✓

Problem:
- Both DACs respond to address 0x60
- Second DAC never gets addressed
- Serial shows only one DAC detected

Fix: Connect DAC #2's A0 to VCC (5V)
```

### ❌ Mistake 3: Both A0 Connected to VCC

```
MCP4725 #1: A0 ──► VCC ✓
MCP4725 #2: A0 ──► VCC ✓

Problem:
- Both DACs respond to address 0x61
- First DAC never gets addressed
- Serial shows only one DAC detected

Fix: Connect DAC #1's A0 to GND
```

### ❌ Mistake 4: A0 Connected to SDA or SCL

```
MCP4725 A0 ──► Pin 18 (SDA)  ✗
or
MCP4725 A0 ──► Pin 19 (SCL)  ✗

Problem:
- DAC address changes during I2C communication
- Erratic behavior, timing-dependent failures
- Intermittent addressing errors

Fix: Connect A0 only to GND or VCC, never to data lines
```

---

## Verification Checklist

Before powering on, verify:

- [ ] **DAC #1 A0 Pin**
  - [ ] Wire goes from A0 pin to GND
  - [ ] Wire is NOT floating
  - [ ] Wire is NOT connected to SDA/SCL

- [ ] **DAC #2 A0 Pin**
  - [ ] Wire goes from A0 pin to VCC (5V)
  - [ ] Wire is NOT floating
  - [ ] Wire is NOT connected to SDA/SCL

- [ ] **I2C Bus (Both DACs)**
  - [ ] SDA pins connected to Teensy Pin 18
  - [ ] SCL pins connected to Teensy Pin 19
  - [ ] Same SDA/SCL line goes to BOTH DACs (shared bus)

- [ ] **Power (Both DACs)**
  - [ ] VCC connected to 5V
  - [ ] GND connected to Teensy GND
  - [ ] Common ground (all GND pins tied together)

---

## Success Signal

When you upload firmware and open serial monitor at 115200 baud:

```
✓ CORRECT OUTPUT:
MCP4725 DAC1 initialized at address 0x60
MCP4725 DAC2 initialized at address 0x61

✗ WRONG OUTPUT:
MCP4725 DAC1 not found!
or
MCP4725 DAC2 not found!
or
Only one DAC detected (should be two)
```

If you see ✗, re-check your A0 wiring using this guide.

---

## Advanced: Why This Works

The I2C protocol uses **7-bit addressing** where each device has a unique address:

```
Device Type          Base Address    A0 Bit
────────────────────────────────────────────
MCP4725              0110000_        ← A0 selects 0 or 1

Result:
0110000 + 0 = 0x60 (when A0=GND)
0110000 + 1 = 0x61 (when A0=VCC)
```

When Teensy sends "Hello 0x60", only the DAC with A0=GND responds.
When Teensy sends "Hello 0x61", only the DAC with A0=VCC responds.

Both DACs can coexist on one 2-wire I2C bus because they have different addresses! 🎉

---

## Related Documentation

- `HARDWARE_WIRING.md` - Physical wiring details
- `MCP4725_SETUP_GUIDE.md` - Complete setup instructions
- MCP4725 Datasheet Section 5.1 - Address pin configuration
