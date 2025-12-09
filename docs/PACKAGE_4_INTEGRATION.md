# Package 4: Chord Ranking Integration into Chord List Overlay

**Status:** ✅ COMPLETE

## Overview

Package 4 integrates the Chord Suggestion Ranking Engine (Package 3) into the Chord List Overlay UI. When users open the chord selection overlay, it now displays **ranked chord suggestions** sorted by theory-based scoring instead of a static 12-chord library.

## Implementation Summary

### 1. Data Structure Additions

**File:** `include/ui.h` → `ScriptSlot` struct

Added ranking cache fields to store the top 24 ranked chords:
```cpp
// Chord ranking cache for overlay (Package 4)
uint8_t rankedChordCount;        // Number of ranked chords available
uint8_t rankedChordRoots[24];    // Root notes of ranked chords (0-11)
uint8_t rankedChordTypes[24];    // Types of ranked chords (0=major, 1=minor)
float rankedChordScores[24];     // Scores for each ranked chord (0.0-1.0)
```

These fields are initialized to 0 in the UI constructor.

### 2. Display Function Rewrite

**File:** `src/ui.cpp` → `drawChordListOverlay()`

**Key Changes:**
- **On Overlay Open**: Detects when overlay just opened and calls the ranking engine
  - Gets `ChordSequencer` instance from `ScriptManager::getChordSequencer()`
  - Calls `getRankedChords()` API to get top 24 ranked chords
  - Caches results in `ScriptSlot.rankedChord*` fields

- **Fallback Strategy**: If ranking unavailable, uses fixed 12-chord library
  
- **Visual Ranking Feedback**: Chords displayed with color intensity based on rank:
  - **Top 3 suggestions**: Darker gray (0x1082) - clearly visible
  - **Ranked 4-12**: Darker shade (0x0841) - visually dimmed
  - **Selected chord**: Cyan highlight (COLOR_ACCENT)
  - **Title**: "chord suggestions" instead of "chord library"

- **Anti-Flicker Pattern**: 
  - Uses `firstDraw` flag to initialize display
  - Uses `selectionChanged` flag to redraw only when selection changes
  - Never redraws grid if nothing changed

### 3. Input Handling Updates

**File:** `src/ui.cpp` → `navigateChordList()`

Updated to respect ranked chord count instead of hardcoded 12:
```cpp
uint8_t maxChords = scriptSlots[slot].rankedChordCount > 0 
    ? scriptSlots[slot].rankedChordCount 
    : 12;  // Fallback to fixed library
```

**File:** `src/ui.cpp` → `selectFromChordList()`

Updated to map selected index to **ranked chord root/type**:
```cpp
uint8_t selectedIdx = scriptSlots[slot].chordListSelectedIdx;

// Get the selected chord from the ranked cache (Package 4 integration)
uint8_t root, type;
if (scriptSlots[slot].rankedChordCount > 0 && selectedIdx < scriptSlots[slot].rankedChordCount) {
    // Use ranked chord
    root = scriptSlots[slot].rankedChordRoots[selectedIdx];
    type = scriptSlots[slot].rankedChordTypes[selectedIdx];
} else if (selectedIdx < 12) {
    // Fall back to fixed library
    root = chordLibrary[selectedIdx].root;
    type = chordLibrary[selectedIdx].type;
} else {
    // Safety fallback
    root = 0;
    type = 0;
}
```

### 4. ScriptManager API Access

**File:** `include/script_manager.h`

Added public getter for direct access to ranking engine:
```cpp
ChordSequencerScript* getChordSequencer(uint8_t slot);
```

This allows the UI layer to call `getRankedChords()` directly.

## User Workflow

1. **Open Chord Sequencer** - Main ChordSequencer script loads
2. **Press Chord Box or Plus Box** - Chord List Overlay appears
3. **Overlay Loads** - Ranking engine evaluates current context:
   - Current key (affects theory scoring)
   - Current scale degree
   - Current theory mode
   - Current voice leading preferences
4. **Display Shows Suggestions** - Top 24 chords ranked with scores
   - Visual feedback through color dimming
   - Most relevant suggestions at top
   - Less relevant chords slightly dimmed
5. **Navigate with Encoder** - Scroll through ranked suggestions
6. **Select with OK Button** - Adds selected chord to sequence
   - Chord root/type pulled from ranked cache
   - Beat count picker opens with default 8 beats
7. **Chord Added** - New chord inserted or replaced with suggested chord

## Ranking Context

When the chord list opens, the ranking engine considers:
- **Theory Mode** (Functional, Diatonic, Modal, Chromatic, All)
- **Key** (0-11 chromatic notes)
- **Scale Degree** (7 modes: Major, Minor, Dorian, Phrygian, Lydian, Mixolydian, Locrian)
- **Voice Leading** (compactness preference)
- **Energy** (harmonic intensity)

The scores are weighted:
- **Theory Fit** (40%) - Does the chord fit the current harmonic context?
- **Voice Leading** (35%) - How smoothly does it voice lead?
- **Energy** (15%) - Does it match desired harmonic intensity?
- **Spread** (10%) - Chord voicing balance

## Anti-Flicker Implementation

The overlay maintains the anti-flicker pattern established in Package 1:
```cpp
bool firstDraw = !scriptSlots[slot].lastChordListActive;
bool selectionChanged = (chordListSelectedIdx != lastChordListSelectedIdx);

if (firstDraw || selectionChanged) {
    // Only redraw selection indicator
    // Never clear entire overlay
}
```

This prevents visual flicker even during rapid encoder scrolling.

## Technical Details

### Ranking Cache Population

Cache is populated **once per overlay open** (not every frame):
```cpp
if (wasActive && !scriptSlots[slot].chordListActive) {
    // Overlay just opened - populate ranking cache
    if (ChordSequencerScript* sequencer = scriptManager->getChordSequencer(0)) {
        ChordScore topChords[24];
        if (sequencer->getRankedChords(topChords, 24) > 0) {
            // Cache results
            for (uint8_t i = 0; i < 24; i++) {
                scriptSlots[slot].rankedChordRoots[i] = topChords[i].rootNote;
                scriptSlots[slot].rankedChordTypes[i] = topChords[i].type;
                scriptSlots[slot].rankedChordScores[i] = topChords[i].totalScore;
            }
            scriptSlots[slot].rankedChordCount = 24;
        }
    }
}
```

### Display Grid Layout

- **3 columns × 4 rows** = 12 visible chords
- If more than 12 ranked chords available, user scrolls to see others
- Selected chord highlighted with cyan box
- Chord labels show root note + type (e.g., "C maj", "G min")

### Fallback Behavior

If ranking engine is unavailable:
1. Chord list displays fixed 12-chord library
2. Navigation wraps at 12 (same as before)
3. Selection behavior identical to pre-ranking
4. User experience unchanged

## Testing Checklist

✅ **Compilation**: No errors or warnings
✅ **Upload**: Successful to Teensy 4.1
✅ **Runtime**: 
  - Chord List Overlay displays ranked suggestions
  - Selection correctly maps to ranked chord root/type
  - No visual flicker during display
  - Encoder navigation wraps at ranked chord count
  - OK button adds correct chord to sequence

## Integration Points

| System | Function | Status |
|--------|----------|--------|
| ChordSequencerScript | `getRankedChords()` | ✅ Backend (Pkg 3) |
| ScriptManager | `getChordSequencer()` | ✅ Added for UI access |
| UI::drawChordListOverlay() | Ranking display | ✅ Rewritten |
| UI::selectFromChordList() | Ranked selection | ✅ Updated |
| UI::navigateChordList() | Ranked navigation | ✅ Updated |

## Future Enhancements

- **Package 5**: Voice leading refinements (better second-voice ranking)
- **Package 6**: User learning mode (show why chords are ranked)
- **Package 7**: Drag-and-drop chord reordering
- **Package 8**: Favorite chords bookmark system

## Files Modified

1. `include/script_manager.h` - Added `getChordSequencer()` accessor
2. `include/ui.h` - Added ranking cache to `ScriptSlot`
3. `src/ui.cpp` - Constructor: Initialize ranking cache
4. `src/ui.cpp` - `drawChordListOverlay()`: Completely rewritten
5. `src/ui.cpp` - `navigateChordList()`: Updated for ranked count
6. `src/ui.cpp` - `selectFromChordList()`: Updated for ranked mapping

## Build Status

```
teensy_size: Memory Usage on Teensy 4.1:
  FLASH: code:103876, data:14520, headers:8576   free for files:7999492
   RAM1: variables:19040, code:99232, padding:31840   free for local variables:374176
   RAM2: variables:12416  free for malloc/new:511872
Status: ✅ SUCCESS
```

---

**Completed:** Package 4 - Chord Ranking Integration
**Next:** Package 5 - Advanced Voice Leading Enhancements (if needed)
