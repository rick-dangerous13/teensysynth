# DAC8568 Migration Guide

## Overview
Successfully migrated from 2x MCP4725 12-bit I2C DACs to 1x DAC8568 16-bit 8-channel SPI DAC.

## Hardware Changes

### Old Configuration (MCP4725)
- **Quantity**: 2 separate DAC modules
- **Resolution**: 12-bit (4,096 steps, 0-4095)
- **Interface**: I2C (SDA=18, SCL=19)
- **Addresses**: 0x60 (CV), 0x61 (Gate)
- **Channels**: 2 (1 CV + 1 Gate)
- **Voltage Range**: 0-5V

### New Configuration (DAC8568)
- **Quantity**: 1 DAC chip
- **Resolution**: 16-bit (65,535 steps, 0-65535)
- **Interface**: SPI (MOSI=11, MISO=12, SCK=13)
- **Chip Select**: Pin 14 (DAC_CS)
- **Channels**: 8 independent outputs (VOUTA-H)
- **Voltage Range**: 0-5V (with 5V VREFIN)

## Wiring Changes

### DAC8568 Pin Connections
```
DAC8568 Pin    Teensy 4.1    Notes
-----------    ----------    -----
VDD     →      5V            Power supply
VSS     →      GND           Ground
VREFIN  →      5V            External reference
VREFOUT →      NC            Leave unconnected
SCLK    →      Pin 13        SPI Clock (shared)
DIN     →      Pin 11        SPI MOSI (shared)
SYNC    →      Pin 14        SPI CS (dedicated)
LDAC    →      GND           Tied low for immediate updates
CLR     →      5V            Tied high (never clear)
DOUT    →      NC            Leave unconnected
```

### 8-Channel Output Mapping
| Channel | Pin   | Primary Use                | Secondary Use     |
|---------|-------|----------------------------|-------------------|
| 0 (A)   | VOUTA | Poliquencer Step 1 CV      | LFO output        |
| 1 (B)   | VOUTB | Poliquencer Step 2 CV      | -                 |
| 2 (C)   | VOUTC | Poliquencer Step 3 CV      | -                 |
| 3 (D)   | VOUTD | Poliquencer Step 4 CV      | -                 |
| 4 (E)   | VOUTE | Poliquencer Step 5 CV      | -                 |
| 5 (F)   | VOUTF | Poliquencer Step 6 CV      | -                 |
| 6 (G)   | VOUTG | Poliquencer Step 7 CV      | -                 |
| 7 (H)   | VOUTH | Poliquencer Step 8 CV      | Gate (optional)   |

## Software Architecture Changes

### 1. New DAC8568 Driver (`include/dac8568.h`, `src/dac8568.cpp`)
Custom driver implementation with:
- `begin()` - Initialize SPI and reset DAC
- `setChannel(channel, value)` - Set 16-bit value on specific channel
- `setVoltage(channel, voltage)` - Set voltage (0.0-5.0V) on channel
- `setAllChannels(value)` - Set all channels to same value
- `setAllVoltages(voltage)` - Set all channels to same voltage
- `voltageToDACValue(voltage)` - Convert voltage to 16-bit value

### 2. ScriptManager Changes
**File**: `include/script_manager.h`, `src/script_manager.cpp`

Added shared DAC management:
```cpp
// Private members
DAC8568* dac;
bool dacInitialized;

// In begin()
dac = new DAC8568(DAC_CS, DAC_MAX_VOLTAGE);
dac->begin();

// When loading scripts
lfoInstances[slot]->setDAC(dac);
poliquencerInstances[slot]->setDAC(dac);
```

### 3. LFO Script Changes
**Files**: `include/lfo_script.h`, `src/lfo_script.cpp`

- Replaced `Adafruit_MCP4725 dac` with `DAC8568* dac` pointer
- Added `setDAC(DAC8568* dacPtr)` method
- Uses channel 0 (DAC_CH_STEP1) for output
- Removed Wire.h dependency (no longer uses I2C)

### 4. Poliquencer Script Changes
**Files**: `include/poliquencer_script.h`, `src/poliquencer_script.cpp`

Major architectural change:
- Replaced 2x MCP4725 (`dacCV`, `dacGate`) with single `DAC8568* dac` pointer
- **Polyphonic output**: Each step outputs continuously to its own channel
- Channels 0-7 output CV for steps 1-8 simultaneously
- Gate output currently disabled (all channels for CV)
- Added `setDAC(DAC8568* dacPtr)` method
- In `setDAC()`: Initializes all 8 channels with step voltages
- In `outputCV()`: Updates only the current step's channel

## Configuration Changes

### config.h Updates
```cpp
// New DAC CS pin
#define DAC_CS        14

// Updated DAC specifications
#define DAC_MAX_VALUE 65535     // 16-bit (was 4095)
#define DAC_MAX_VOLTAGE 5.0f    // 5V reference

// Channel definitions
#define DAC_CH_STEP1    0  // Poliquencer step 1 / LFO
#define DAC_CH_STEP2    1  // Poliquencer step 2
// ... through ...
#define DAC_CH_STEP8    7  // Poliquencer step 8 / Gate
```

Removed MCP4725 I2C addresses (no longer needed).

## New Capabilities

### 1. Polyphonic Sequencing
The Poliquencer now outputs **all 8 steps simultaneously** on separate channels:
- Each step continuously outputs its CV voltage
- All steps are "active" at once
- Perfect for:
  - True 8-voice polyphony
  - Multi-parameter modulation (control 8 different synth parameters)
  - Parallel sequence patterns
  - Complex modulation matrices

### 2. Higher Resolution
- 16-bit resolution = 65,535 steps (was 4,096)
- **16x more precise** pitch control
- Smoother LFO waveforms
- More accurate voltage tracking

### 3. Faster Updates
- SPI is faster than I2C
- No address conflicts or bus contention
- All 8 channels can update rapidly

## Testing Checklist

When you receive your DAC8568:

### Hardware Test
- [ ] Connect DAC8568 per wiring guide
- [ ] Verify 5V on VDD and VREFIN
- [ ] Check LDAC tied to GND, CLR tied to 5V
- [ ] Upload firmware and monitor serial output
- [ ] Confirm "DAC8568 initialized successfully" message

### LFO Test
- [ ] Load LFO script from library
- [ ] Verify output on VOUTA (channel 0)
- [ ] Test all waveforms (sine, triangle, square, sawtooth)
- [ ] Measure voltage range with multimeter (should be 0-5V)
- [ ] Verify smooth waveform transitions

### Poliquencer Test
- [ ] Load Poliquencer from library
- [ ] Verify CV output on all 8 channels (VOUTA-H)
- [ ] Check that all steps output continuously
- [ ] Test step value editing - verify voltage changes on corresponding channel
- [ ] Measure voltages with multimeter while sequencer runs
- [ ] Verify 1V/octave scaling (semitone changes = ~83mV steps)

### Integration Test
- [ ] Test with actual Eurorack modules
- [ ] Verify polyphonic patches work with 8-voice capability
- [ ] Check that touch controls still work correctly
- [ ] Test script switching (LFO ↔ Poliquencer)

## Known Limitations

1. **Gate Output**: Currently disabled in Poliquencer to dedicate all 8 channels to step CVs. Options:
   - Use CV amplitude/voltage thresholds as gate indication
   - Add external gate output hardware
   - Reserve one channel specifically for gate

2. **Shared DAC**: Only one DAC instance exists. Scripts must coordinate channel usage:
   - LFO uses channel 0
   - Poliquencer uses all 8 channels
   - Running both simultaneously will conflict on channel 0

3. **No Persistence**: DAC8568 doesn't have EEPROM. Values reset on power cycle (same as MCP4725 when used without EEPROM writes).

## Rollback Procedure

If needed to revert to MCP4725:
1. Checkout previous git commit before DAC8568 changes
2. Reconnect MCP4725 modules per old wiring guide
3. Rebuild and upload firmware

## Files Modified

### Created
- `include/dac8568.h` - DAC8568 driver header
- `src/dac8568.cpp` - DAC8568 driver implementation
- `docs/DAC8568_MIGRATION.md` - This document

### Modified
- `include/config.h` - DAC pin and value definitions
- `include/script_manager.h` - Added DAC8568 instance
- `src/script_manager.cpp` - Initialize and share DAC
- `include/lfo_script.h` - Changed to use DAC8568 pointer
- `src/lfo_script.cpp` - Updated DAC interface calls
- `include/poliquencer_script.h` - Changed to use DAC8568 pointer
- `src/poliquencer_script.cpp` - Major refactor for 8 channels
- `HARDWARE_WIRING.md` - Complete rewrite of DAC section
- `.github/copilot-instructions.md` - Updated hardware specs

## Future Enhancements

### Possible Improvements
1. **Dynamic channel allocation** - Scripts request channels from ScriptManager
2. **Channel mixing** - Multiple scripts share channels with priority/mixing
3. **Gate encoding** - Use velocity or separate digital outputs for gates
4. **Calibration** - Per-channel voltage calibration and offset adjustment
5. **Quantization** - Musical scale quantization per channel
6. **Modulation routing** - Matrix for routing LFOs to sequencer steps

### Hardware Additions
- External gate outputs (8x transistor switches or digital outputs)
- Input CV for external modulation sources
- Second DAC8568 for 16-channel capability

## Support

For issues or questions:
1. Check serial monitor for initialization messages
2. Use multimeter to verify output voltages
3. Test individual channels with simple test script
4. Refer to DAC8568 datasheet for electrical specs

## Conclusion

The migration to DAC8568 provides:
- ✓ 8x more CV outputs (8 vs 2)
- ✓ 16x higher resolution (16-bit vs 12-bit)
- ✓ True polyphonic capability
- ✓ Faster SPI interface
- ✓ Cleaner wiring (1 chip vs 2 modules)
- ✓ Lower cost (1 chip vs 2 DAC modules)

Build succeeded with no warnings. Ready for hardware testing!
