# MCP4725 Quick Start Card

**Print this or keep it open while wiring!**

---

## 🎯 ONE PAGE CHEAT SHEET

### What You're Building
Two separate I2C DACs on one 2-wire bus, each with unique address.

### Your MCP4725 Pinout
Your modules have pins labeled: **OUT, GND, SCL, SDA, VCC**

(Different boards label pins differently - verify against your board!)

### The Magic: A0 Pin
```
DAC #1: A0 → GND   = Address 0x60
DAC #2: A0 → VCC   = Address 0x61
```

**NOTE:** You need to find the A0 pin on your modules (may be small solder pad or unlabeled pin). Check your board documentation or look for a pad labeled "A0" or "ADDR".

### Wiring (Copy This Exactly)

**Your MCP4725 Pin Labels: OUT, GND, SCL, SDA, VCC**

**MCP4725 #1:**
```
OUT → TRRS Jack 1, Ring 1
GND → GND
SCL → Pin 19
SDA → Pin 18
VCC → 5V
A0  → GND ← THIS IS KEY (find this pin on your board!)
```

**MCP4725 #2:**
```
OUT → TRRS Jack 2, Ring 2
GND → GND
SCL → Pin 19          ← SAME as #1!
SDA → Pin 18          ← SAME as #1!
VCC → 5V
A0  → VCC (5V) ← THIS IS KEY (find this pin on your board!)
```

**TRRS Jack:**
```
Sleeve (bottom) = GND
Ring 1 = CV Out 1
Ring 2 = CV Out 2
Tip (top) = GND
```

---

## ✅ Verification Checklist

Before power-on:

- [ ] Both DACs have 5V power (VCC)
- [ ] Both DACs share ground (GND)
- [ ] Both DACs connect to Pin 18 (SDA) - YES, SAME PIN
- [ ] Both DACs connect to Pin 19 (SCL) - YES, SAME PIN
- [ ] DAC #1 A0 → GND (use a wire, not floating!)
- [ ] DAC #2 A0 → VCC/5V (use a wire, not floating!)
- [ ] DAC #1 OUT → TRRS Ring 1
- [ ] DAC #2 OUT → TRRS Ring 2
- [ ] TRRS Sleeve → GND

---

## 🚀 Upload & Test

1. Plug in Teensy via USB
2. Upload firmware: `pio run --target upload`
3. Open Serial Monitor (115200 baud)

### Expected Output ✓
```
MCP4725 DAC1 initialized at address 0x60
MCP4725 DAC2 initialized at address 0x61
```

### If You See Error ✗
```
"MCP4725 not found!"
```
→ Check A0 wiring (is it connected to GND or VCC, not floating?)

---

## 🔧 Troubleshooting in 30 Seconds

| Problem | Check This |
|---------|-----------|
| DAC #1 not found | Is A0 on DAC #1 connected to GND? |
| DAC #2 not found | Is A0 on DAC #2 connected to VCC? |
| Both not found | Are pins 18 & 19 connected to both DACs? |
| Only 1 DAC detected | Check BOTH A0 pins (one to GND, one to VCC) |

---

## 📊 How It Works (Simple Version)

```
Teensy talks on I2C (pins 18, 19)
         ↓
    [Hello 0x60]  → DAC #1 says "that's me!" ✓
         ↓
    [Hello 0x61]  → DAC #2 says "that's me!" ✓

Both listen on same wires, respond to different addresses.
```

---

## 🎵 Success = Both DACs Working!

Once verified working:
- Rotate encoder → CV output changes on TRRS Jack
- Multimeter test → Voltage sweeps 0-5V
- Plug into Eurorack → CV controls your synth!

---

**Still stuck?** Read:
- `MCP4725_SETUP_GUIDE.md` (detailed)
- `MCP4725_ADDRESS_GUIDE.md` (super detailed)
- `HARDWARE_WIRING.md` (full reference)
