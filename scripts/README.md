# TeensySynth Scripts

This directory contains built-in and user scripts for the TeensySynth.

## Built-in Scripts

### 1. LFO (Low-Frequency Oscillator)
**Location:** Built into firmware  
**Description:** Generates low-frequency oscillation for CV modulation  
**Output:** MCP4725 DAC (0-5V)  

**Features:**
- **Waveforms:** Sine, Triangle, Square, Sawtooth
- **Frequency Range:** 0.01 Hz to 100 Hz
- **Output Level:** 0V to 5V (adjustable)
- **CV Output:** Via MCP4725 12-bit DAC

**Hardware Requirements:**
- MCP4725 DAC module connected to I2C
  - SDA → Pin 18
  - SCL → Pin 19
  - VCC → 3.3V
  - GND → GND
  - OUT → Your CV destination (0-5V)

**Parameters:**
- Frequency (encoder to adjust)
- Level/Amplitude (encoder to adjust)
- Waveform selection

### Future Scripts (Coming Soon)

2. **Sequencer** - Step sequencer with CV/Gate output
3. **Envelope** - ADSR envelope generator
4. **Clock** - Clock divider/multiplier

## Adding Custom Scripts

In future versions, you'll be able to add custom scripts by:

1. Creating a `.lua` or `.scd` (SuperCollider) file
2. Placing it in this `scripts/` directory
3. The script will appear in the script library browser

### Script Structure (Planned)

```lua
-- metadata
name = "My Script"
author = "Your Name"
version = "1.0"

-- initialization
function init()
  -- setup code
end

-- main loop (called continuously)
function update()
  -- runtime code
end

-- cleanup
function cleanup()
  -- shutdown code
end
```

## Hardware Connections

### MCP4725 DAC (CV Output)
```
MCP4725        Teensy 4.1
--------       ----------
VCC     →      3.3V
GND     →      GND
SDA     →      18 (SDA)
SCL     →      19 (SCL)
OUT     →      CV destination (0-5V)
```

**Important Notes:**
- The MCP4725 outputs 0-VCC voltage (with VCC at 3.3V or 5V)
- For full 0-5V range, power the MCP4725 with 5V
- Connect to Eurorack CV inputs (typically 0-5V or 0-10V range)
- Multiple DACs can be added by using different I2C addresses

## Script Slots

The TeensySynth supports **4 concurrent script slots**, allowing you to run up to 4 scripts simultaneously in parallel. Each script:
- Runs independently
- Has its own display quadrant
- Can generate audio/CV output
- Updates in real-time

## Usage

1. From the main menu, select **SCRIPTS**
2. Choose a slot (1-4) by turning the encoder
3. Press **OK** to browse available scripts
4. Select a script from the library
5. Press **OK** to load it into the slot
6. The script starts running immediately

The script display shows:
- Script name
- Current parameters
- Real-time status

Press **BACK** to return to previous menus.
