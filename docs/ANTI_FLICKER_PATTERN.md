# Anti-Flicker Display Pattern for Teensysynth

## Problem Summary
Controls disappearing or flickering when:
1. First appearing on screen
2. Returning from overlays/modals
3. Values changing via encoder
4. Selection states toggling

## Root Causes
- **Full redraws every frame**: Redrawing unchanged elements causes flicker
- **Missing first-frame initialization**: State tracking initialized to 0, can't detect "first draw"
- **Late transition detection**: Detecting overlay closure after drawing instead of before
- **No per-element state tracking**: Using function-scope flags that reset or don't persist correctly

## Solution Pattern

### 1. Add State Tracking to ScriptSlot Struct
Store a per-element redraw flag that persists across frames:

```cpp
// In include/ui.h, struct ScriptSlot:
bool elementNeedsRedraw;  // Force full redraw on next frame
```

Initialize in `ui.cpp` constructor:
```cpp
scriptSlots[i].elementNeedsRedraw = true;  // Force initial draw
```

### 2. Three-Tier Redraw Logic in Drawing Function

```cpp
void UI::drawElement(uint8_t slot, ...) {
    // TIER 1: Check if full redraw needed
    bool isFirstDraw = scriptSlots[slot].elementNeedsRedraw;
    
    for (int i = 0; i < numElements; i++) {
        // TIER 2: Check if selection state changed
        bool isSelected = (scriptSlots[slot].selectedElement == i);
        bool wasSelected = (scriptSlots[slot].lastSelectedElement == i);
        bool selectionStateChanged = (isSelected != wasSelected);
        
        // TIER 3: Check if value actually changed
        bool valueChanged = (scriptSlots[slot].elementValue != scriptSlots[slot].lastElementValue);
        
        // Only redraw if ANY condition is true
        bool needsRedraw = isFirstDraw || selectionStateChanged || (valueChanged && isSelected);
        
        if (!needsRedraw) continue;
        
        // Draw element
        drawElement(i, ...);
    }
    
    // Update tracking AFTER drawing
    scriptSlots[slot].lastSelectedElement = scriptSlots[slot].selectedElement;
    scriptSlots[slot].lastElementValue = scriptSlots[slot].elementValue;
    
    // CRITICAL: Clear flag AFTER drawing
    scriptSlots[slot].elementNeedsRedraw = false;
}
```

### 3. Detect Transitions at Frame Start

In the main drawing function (e.g., `drawChordSequencer`), detect state transitions **early**, before any drawing:

```cpp
// EARLY in function, before drawing anything
bool overlayJustClosed = (!overlayActive && overlayWasActive);

// Set redraw flags for affected elements
if (overlayJustClosed || firstDraw) {
    scriptSlots[slot].elementNeedsRedraw = true;
}

// Later, drawing functions will see this flag set
```

**Key timing:**
- ✅ Set flag at frame start (line ~910)
- ✅ Draw functions check flag and clear it (line ~800)
- ✅ Flag persists across multiple function calls

### 4. Float Tolerance for Value Changes

For continuous values (0.0-1.0 ranges), use tolerance to avoid precision flicker:

```cpp
const float TOLERANCE = 0.001f;  // Adjust per parameter
bool valueChanged = (fabs(current - last) > TOLERANCE);
```

### 5. Never Use These Anti-Patterns

❌ **Static arrays at file scope**: Can't be reset per-slot transition
```cpp
// DON'T DO THIS
static bool firstDraw[MAX_SCRIPTS] = {0};  // Scope problems
```

❌ **Resetting flags every frame**: Causes constant redraws
```cpp
// DON'T DO THIS
if (someCondition) {
    element.needsRedraw = false;  // Resets unconditionally every frame
}
```

❌ **Late transition detection**: After other state updates
```cpp
// DON'T DO THIS
// ... lots of drawing ...
if (overlayJustClosed) {  // Too late! Already drew with stale state
    element.needsRedraw = true;
}
```

❌ **Clearing flag before drawing**: Causes skipped frames
```cpp
// DON'T DO THIS
element.needsRedraw = false;
drawElement();  // Already cleared, so this frame won't redraw
```

## Implementation Checklist

When adding new UI elements:

- [ ] Add `lastXxxValue` fields for all displayed values
- [ ] Add `lastSelectedElement` to track selection changes
- [ ] Add `elementNeedsRedraw` flag to ScriptSlot
- [ ] Initialize `elementNeedsRedraw = true` in constructor
- [ ] **At frame start** (line ~910 in `drawChordSequencer`):
  - [ ] Calculate transition flags (`overlayJustClosed`, etc.)
  - [ ] Set `element.needsRedraw = true` on transitions
- [ ] **In drawing function** (new or existing):
  - [ ] Calculate `isFirstDraw` from slot flag
  - [ ] Implement three-tier logic: firstDraw || selectionChanged || (valueChanged && isSelected)
  - [ ] Skip drawing with `if (!needsRedraw) continue;`
  - [ ] Update `last*` values AFTER drawing
  - [ ] **Clear flag AFTER loop**: `element.needsRedraw = false;`
- [ ] Use float tolerance for continuous values
- [ ] Test on hardware: no flicker, no disappearing elements

## Global Parameter Example (Working Solution)

See `drawGlobalParameterBoxes()` in `src/ui.cpp` for reference:
- Lines 798-805: State/flag initialization
- Lines 815-847: Three-tier redraw logic
- Lines 858-866: Update tracking and clear flag
- Lines 912-915: Frame-start transition detection

## Key Principle

**State is truth. Drawing is consequence.**

- Store state in `ScriptSlot` (truth)
- Detect state changes early in frame
- Set redraw flags before drawing
- Drawing functions check flags and only redraw when needed
- Clear flags after drawing completes

This ensures:
- ✅ No flickering (selective redraw)
- ✅ No disappearing elements (proper initialization and transitions)
- ✅ Smooth experience (minimal redraws)
- ✅ Easy to debug (state clearly tracks what should draw)
