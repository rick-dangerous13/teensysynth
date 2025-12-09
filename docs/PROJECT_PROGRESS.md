# Teensysynth Project Progress

## Completed Packages

### ✅ Package 1: Global Parameters (UI Layer)
- 4 global parameter boxes below chord progression
- Display: Key (C-B), Theory Mode, Compactness, Energy
- Selection highlighting (cyan), editing highlighting (yellow)
- Encoder navigation + OK/BACK button control

### ✅ Package 2: Per-Chord Parameters (Beat Count Picker)
- 2-row layout: beats (top), inversion/spread/theory (bottom)
- Beat count in FONT_LARGE, vertically centered
- 5 navigable items: beats → inversion → spread → theory → done
- OK button enters edit mode when parameter selected, closes picker when "done"
- Navigation hint moved outside white box at screen bottom

### ✅ Package 3: Chord Suggestion Ranking Engine (Backend)
- Scores chords based on:
  - Theory Mode (Functional, Diatonic, Modal, Chromatic, All)
  - Energy (0.0-1.0 harmonic tension)
  - Voice Leading (0.0-1.0 smoothness preference)
  - Spread (placeholder for future enhancement)
- API Functions:
  - `scoreChord(root, type, prevRoot, prevType)` → single score
  - `getRankedChords(outCount)` → sorted array of 24 chords
- Theory rules implemented:
  - FUNCTIONAL: I=1.0, V=0.9, IV=0.8, ii/vi=0.5, others=0.3
  - DIATONIC: In-scale=0.9, chromatic=0.2
  - MODAL: In-scale=0.85, neighbors=0.4, others=0.1
  - CHROMATIC: In-scale=0.6, others=0.5
  - ALL: Everything=0.5
- Voice leading calculation:
  - Considers semitone distance between roots
  - Weighted by voiceLeadingCompactness parameter (0.0-1.0)
  - Same type chords preferred slightly
- Sorting: Bubble sort by totalScore descending
- Memory: 576 bytes for 24 scores
- Performance: ~100-200μs sort time

## In Progress: None

## Planned Packages

### Package 4: Theory-Based Chord Suggestions (UI)
- **Goal**: Display ranked chord suggestions instead of fixed 12-chord grid
- **Implementation**:
  - Call `getRankedChords()` in Chord List Overlay
  - Display top 5-10 suggestions
  - Visual ranking: size, brightness, position
  - Score displayed for debugging (optional, remove for release)
- **Status**: Planning phase
- **Dependencies**: Requires Package 3 ✅

### Package 5: Advanced Voice Leading
- **Goal**: Enhance voice leading scoring
- **Features**:
  - Remember all previous chords in progression, not just last one
  - Detect and reward ii-V-I patterns
  - Avoid parallel movement detection
  - Mode-specific voice leading rules
- **Status**: Not started
- **Dependencies**: Requires Package 4 ✅

### Package 6: User Customization
- **Goal**: Allow tuning ranking weights and rules
- **Features**:
  - Save/load preference sets
  - Adjust theory weight, energy weight, voice leading weight
  - Toggle theory rules per mode
  - Profile system (Jazz, Classical, Pop, Experimental)
- **Status**: Not started

## Display/UI Anti-Flicker Solution

### ✅ Solution Documented: ANTI_FLICKER_PATTERN.md

**Core Pattern:**
1. Add `needsRedraw` boolean to ScriptSlot struct
2. Initialize `needsRedraw = true` in constructor
3. Detect transitions at frame start (line ~910)
4. Three-tier redraw logic: firstDraw || selectionChanged || (valueChanged && isSelected)
5. Clear flag after drawing: `needsRedraw = false`

**Key Principle**: State is truth. Drawing is consequence.
- Store state in ScriptSlot (persistent across frames)
- Detect state changes early in frame
- Set redraw flags before drawing
- Drawing functions check flags and only redraw when needed
- Clear flags after drawing completes

**Applied to:**
- ✅ Global parameter boxes (Package 1)
- ✅ Per-chord parameters in Beat Count Picker (Package 2)
- ✅ Button strip at bottom of screens

**Never use:**
- ❌ Static arrays at file scope (scope problems)
- ❌ Resetting flags unconditionally every frame (constant redraws)
- ❌ Late transition detection (after drawing already happens)
- ❌ Clearing flag before drawing (skips frames)

## Hardware Configuration

- **MCU**: Teensy 4.1 (600MHz ARM Cortex-M7)
- **Display**: ILI9341 240x320 TFT (SPI)
- **Touch**: XPT2046 touchscreen (SPI, polling)
- **Input**: Rotary encoder + button + touchscreen
- **Output**: DAC8568 16-bit 8-channel DAC for CV
- **Build**: PlatformIO, teensy41 environment
- **Current Memory**: 102,788 / 8,009,024 bytes FLASH (1.3% used)

## Lessons Learned

1. **Display Flickering**: Not about redrawing faster, it's about redrawing less. Use selective redraw based on state changes.

2. **State Tracking**: Always track `last*` versions of displayed values. Compare against last frame to detect changes.

3. **Transition Detection**: Detect state transitions (overlay closing, selection changing) at frame start, before drawing.

4. **Float Comparisons**: Use tolerance (e.g., 0.001f) for float values to avoid precision-induced flicker.

5. **Per-Slot State**: Store redraw flags in ScriptSlot struct, not file-scope statics. Allows proper reset when leaving/returning to screen.

6. **Ranking Algorithms**: Start with simple scoring, keep weights adjustable. Bubble sort is fine for small lists (≤100 items).

## Next Steps

1. **Review Package 3**: Verify ranking engine works correctly with different theory modes
2. **Test on Hardware**: Play with settings, verify no unexpected behavior
3. **Plan Package 4**: Design Chord List Overlay changes to use ranked suggestions
4. **Start Package 4**: Implement UI layer calling `getRankedChords()`

## Code Statistics

| Component | Lines | Status |
|-----------|-------|--------|
| Global Parameters (UI) | ~140 | ✅ Complete |
| Beat Count Picker | ~280 | ✅ Complete |
| Ranking Engine (Backend) | ~150 | ✅ Complete |
| Button Strip | ~80 | ✅ Complete |
| Total UI Code | ~3000 | Stable |
| Total Project | ~3500 | Building |

## Files Created/Modified

### New Documentation
- `docs/ANTI_FLICKER_PATTERN.md` - Display pattern guide
- `docs/PACKAGE_3_CHORD_RANKING_ENGINE.md` - Ranking engine reference
- `docs/PACKAGE_3_SUMMARY.md` - Package 3 completion summary
- `docs/PROJECT_PROGRESS.md` - This file

### Code Changes
- `include/chord_sequencer_script.h` - Added ranking API + ChordScore struct
- `src/chord_sequencer_script.cpp` - Added ranking engine implementation
- `include/ui.h` - Added globalParamNeedsRedraw flag to ScriptSlot
- `src/ui.cpp` - Added global parameter drawing, beat count picker, button strip
- `src/script_manager.cpp` - Added global parameter accessors
- `src/main.cpp` - Added encoder handler for global parameters

---

**Last Updated**: December 9, 2025
**Status**: Package 3 Complete, Ready for Package 4
