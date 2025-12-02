# GitHub Copilot Instructions for Polyphonion

## Display Rendering Principles

### Anti-Flicker Strategy
**CRITICAL**: Always implement selective redrawing to prevent screen flicker on ILI9341 displays.

#### Required Pattern for All UI Drawing:
1. **Track state changes**: Store `last*` versions of all displayed values
2. **Compare before drawing**: Only redraw elements that have changed
3. **Clear selectively**: Use targeted `fillRect()` on changed areas, never full-screen clears during updates
4. **Initialize properly**: Set `last*` values to impossible values (e.g., 255) to force first draw

#### Example Implementation:
```cpp
// In struct/class definition:
uint8_t currentStep;
uint8_t lastCurrentStep;  // Always add last* tracking

// In initialization:
lastCurrentStep = 255;  // Force first draw

// In drawing code:
if (currentStep != lastCurrentStep) {
    // Clear only the old indicator
    fillRect(oldX, oldY, width, height, COLOR_BG);
    // Draw new indicator
    fillRect(newX, newY, width, height, COLOR_ACCENT);
    lastCurrentStep = currentStep;
}
```

### Display Architecture Rules

1. **Never redraw unchanged elements** - Always check if value changed before drawing
2. **Use targeted clears** - `fillRect()` only the specific area that changed
3. **Batch related updates** - Group logical drawing operations together
4. **Avoid full-screen operations** - Never `fillScreen()` during normal updates
5. **Parameter change detection** - Use string comparison or flags to detect major changes that require full redraw
6. **Title placement** - All script/screen titles use FONT_SMALL in top-left corner (x+20, y+5)
7. **Separate trigger from value changes** - Don't redraw controls on step/trigger changes unless they need visual feedback
8. **Stable edit controls** - When editing a control, prevent trigger/step animations from causing flicker
9. **Single active highlight** - Only the currently manipulatable control should show highlight box (COLOR_HIGHLIGHT)
10. **Edit mode transitions** - Only redraw when control actually enters/exits edit mode, not on every step change

### Color Palette Standards
- `COLOR_BG` (0x0000) - Background black
- `COLOR_FG` (0xFFFF) - Foreground white
- `COLOR_DIM` (0x39E7) - Dim gray for labels
- `COLOR_ACCENT` (0x07FF) - Cyan for active elements
- `COLOR_HIGHLIGHT` (0xFFE0) - Yellow for selected/editing elements
- Custom colors: Use consistent palette (e.g., Poliquencer uses grayscale + 0x5D9F light blue)

## Code Organization

### Script Types
- **Type 0**: LFO scripts - waveform generators
- **Type 1**: Sequencer scripts - basic 8-step sequencers
- **Type 2**: Poliquencer - advanced sequencer with gate modes and artistic UI

### Adding New Scripts
1. Create header in `include/` and implementation in `src/`
2. Add instance array in `script_manager.h/cpp`
3. Add to script library with unique type number
4. Implement getter/setter methods in ScriptManager
5. Add UI rendering in `ui.cpp` with proper state tracking
6. Add input handling in `main.cpp`

### UI State Management
- Store UI state in `scriptSlots[]` array in UI class
- Sync with ScriptManager using getter/setter methods
- Always track previous state with `last*` variables for selective redrawing

## Hardware Specifications
- **MCU**: Teensy 4.1 (600MHz ARM Cortex-M7)
- **Display**: ILI9341 240x320 TFT (SPI, using ILI9341_t3 library)
- **Input**: Rotary encoder with button
- **Output**: 2x MCP4725 12-bit DACs for CV
- **Shell**: zsh on macOS
- **Build**: PlatformIO with `teensy41` environment

## Testing Requirements
- Always test on hardware before considering complete
- Verify no flickering with encoder interaction
- Check all edit modes cycle correctly
- Confirm CV output changes appropriately

## Common Pitfalls to Avoid
1. ❌ Drawing unchanged elements every frame
2. ❌ Using `fillScreen()` during updates
3. ❌ Not tracking previous state
4. ❌ Fetching stale data after modifying UI state
5. ❌ Nesting conditional logic that excludes valid cases
6. ❌ Drawing text/graphics without checking if value changed
