# MCP4725 Pinout Reference - Your Board Configuration

## Your Module Labels

Your MCP4725 modules have these pin labels (in order):
```
PIN 1: OUT   (Signal output)
PIN 2: GND   (Ground)
PIN 3: SCL   (I2C Clock)
PIN 4: SDA   (I2C Data)
PIN 5: VCC   (Power)
PIN 6: A0    (Address select - may not be labeled!)
```

## Quick Connection Map

| Your Label | Function | Teensy/Power |
|-----------|----------|--------------|
| OUT | Signal output | TRRS Jack Ring |
| GND | Ground | GND |
| SCL | I2C Clock | Pin 19 |
| SDA | I2C Data | Pin 18 |
| VCC | Power | 5V |
| A0 | Address select | GND (DAC #1) or VCC (DAC #2) |

## The A0 Pin (Critical!)

Your board may not have the A0 pin clearly labeled. It's typically:
- A small solder pad near the main IC chip
- Located on one edge of the board
- May be marked as "ADDR", "A0", or have a tiny "A0" silkscreen label
- Could be a through-hole pin if you have a proto version

**If you can't find A0:**
1. Check your board's datasheet/documentation
2. Look for a small resistor pad near the main chip (usually R2 or R3)
3. Email the seller for pinout diagram
4. Post a photo in electronics forums - they can identify it

## Wiring Your Boards

### MCP4725 #1 (Address 0x60)
```
Out  ──► TRRS Jack 1, Ring 1
GND  ──► GND (common)
SCL  ──► Pin 19 (shared I2C clock)
SDA  ──► Pin 18 (shared I2C data)
VCC  ──► 5V Power
A0   ──► GND (this determines address 0x60)
```

### MCP4725 #2 (Address 0x61)
```
Out  ──► TRRS Jack 2, Ring 2
GND  ──► GND (same as #1)
SCL  ──► Pin 19 (same as #1!)
SDA  ──► Pin 18 (same as #1!)
VCC  ──► 5V Power (same rail)
A0   ──► VCC/5V (this determines address 0x61)
```

## TRRS Jack Wiring

**For both jacks (same configuration):**
```
TRRS Jack Connector:
  Sleeve (bottom)  → GND
  Ring 2 (middle)  → CV Out 2 (from DAC #2) [only for Jack 2]
  Ring 1 (middle)  → CV Out 1 (from DAC #1) [only for Jack 1]
  Tip (top)        → GND
```

## Finding Your A0 Pin

### Method 1: Datasheet
Look up "MCP4725" breakout board documentation - should show all pins

### Method 2: Visual Inspection
- Count the pins on your module (typically 6)
- Identify VCC (usually labeled, closest to resistors)
- Find GND (should be obvious)
- Check for any unlabeled pads - A0 is often one of these
- A0 is usually near the main chip (8-pin DIP or smaller)

### Method 3: Check Resistor Markings
Some boards have pull-up resistors on A0:
- If A0 is pulled to VCC: you'll see a resistor going to VCC
- If A0 is floating: you need to wire it explicitly

### Method 4: Trial & Error (Last Resort)
If you can't find A0:
1. Test the board with A0 unconnected - DAC might work at one address
2. Try connecting an unused pin to GND or VCC and check if addressing changes
3. Use I2C address scanner (available in Arduino IDE) to verify

## Common Board Variations

### Adafruit MCP4725 Breakout
```
Typical pin order (may vary):
1: OUT
2: GND
3: SCL
4: SDA
5: VCC
6: A0 (solder pad on back)
```

### Generic Chinese Module
```
Often labeled on silk screen:
OUT, GND, SCL, SDA, VCC, A0 (or unmarked)
```

### Some modules combine pins:
```
Rare: Some boards have 4 pins (OUT, GND, SDA+SCL combined, VCC)
```

## Troubleshooting A0

**Issue: Can't find A0 pin**
- [ ] Check the back of the board
- [ ] Look for small solder pads
- [ ] Check for resistor network with labels
- [ ] Verify with ohmmeter which pad connects to VCC/GND

**Issue: A0 pin is tiny/hard to solder**
- Use 30AWG wire or smaller
- Use very fine-tipped soldering iron (20W or better)
- Apply flux for easier solder flow
- Let solder cool before moving wire

**Issue: Unsure if it's A0 or address pin**
- Check datasheet - should say "A0" explicitly
- Measure with multimeter: A0 should be either high (VCC) or low (GND)
- Not A0 if it's floating to weird voltages

## Success Verification

After wiring, verify:
1. **Power**: 5V between VCC and GND (multimeter test)
2. **I2C bus**: SCL and SDA have pull-up resistors (should read ~3-4V when idle)
3. **A0 pins**: 
   - DAC #1 A0 should read 0V (connected to GND)
   - DAC #2 A0 should read 5V (connected to VCC)
4. **Upload firmware** - should show both DACs detected in serial monitor

---

## Reference Schematics

### Typical MCP4725 Breakout Circuit
```
         ┌─────────────────┐
    VCC─┤1  MCP4725   6│─OUT
    GND─┤2           5│─A0
    SDA─┤3           4│─SCL (internally connected via chip)
    GND─┤4  (connect) │
         └─────────────────┘
    
    With pull-ups:
    SCL ──[10k]── VCC
    SDA ──[10k]── VCC
    A0  ──[10k]── VCC (or to GND for default address)
```

## Next Steps

1. **Locate A0 pin** on both your modules
2. **Double-check pinout** against this guide
3. **Solder carefully** - use proper technique for tiny pads
4. **Verify connections** with multimeter before power-on
5. **Upload firmware** and check serial monitor

Your specific pinout is unusual but should work fine once A0 is properly connected!
