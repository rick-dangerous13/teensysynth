# Polyphonion Codebase Refactoring Analysis

**Generated:** December 9, 2025  
**Current State:** Prototype with significant technical debt  
**Total Lines of Code:** ~8,570  

---

## Executive Summary

Your codebase exhibits classic prototyping patterns that have accumulated technical debt. The main issues are:

1. **Monolithic UI layer** (3,151 lines in ui.cpp—too many responsibilities)
2. **Massive main.cpp** (1,034 lines—mixing state management, I/O, and business logic)
3. **Tight coupling** between UI state and script management
4. **Duplicated state tracking** (UI maintains shadow copies of script data)
5. **Deep include chains** (ui.h → display.h → script_manager.h creates tightly coupled modules)
6. **No clear separation of concerns** (rendering, state management, and input handling mixed)

---

## Detailed Findings

### 1. **UI Layer Fragmentation** (CRITICAL)

**Problem:** `ui.h` (344 lines) + `ui.cpp` (3,151 lines) = 3,495 lines with 6 different responsibilities

**Current responsibilities in UI:**
- Menu navigation and state tracking
- Script slot data duplication (`scriptSlots[MAX_SCRIPTS]` array)
- LFO parameter editing (waveform, frequency, level)
- Sequencer state tracking (edit step, gate modes, durations)
- Chord sequencer management (carousel, chord list, beat picker overlays)
- Chord ranking display and selection
- Display rendering (300+ drawing functions)

**Evidence:**
```cpp
// In ui.h - ScriptSlot struct (lines 23-114)
// Contains 100+ member variables tracking:
//   - LFO state (frequency, level, phase)
//   - Sequencer state (steps, durations, gates, directions)
//   - Chord sequencer state (chords, beats, carousel, overlays)
//   - Global parameters (root, degree, theory mode)
//   - "Last" copies of all above for change detection
```

**Impact:**
- Hard to modify one aspect without affecting others
- Impossible to test UI logic in isolation
- Changes to script state require updating duplicate tracking in UI
- 3,151-line cpp file is unmaintainable

**Example Problem:**
```cpp
// UI maintaining shadow copy of chord data
uint8_t chordRoots[MAX_CHORD_SLOTS];      // UI copy
uint8_t lastChordRoots[MAX_CHORD_SLOTS];  // Last frame's copy
// vs.
// Script manager maintaining same data
// = duplicate state that can drift
```

---

### 2. **main.cpp Over-Concentration** (CRITICAL)

**Problem:** 1,034 lines handling multiple concerns

**Current responsibilities:**
- Global state management (`AppState`, `currentState`, `previousState`)
- Input handling (encoder, buttons, touch)
- State machine logic (5 different states with handlers)
- Script lifecycle management (loading, unloading, parameter updates)
- Specific script parameter editing (LFO, Poliquencer, ChordSequencer)
- Direct integration of I/O → logic → rendering

**Evidence:**
```cpp
// Lines 304-463: handleScriptSelectState() = 160 lines
// Handles: encoder input, button input, touch input, 
// poliquencer editing, chord sequencer editing, state transitions
// All mixed together with no separation

// Lines 261-463: ~200 lines of encoder delta handling
// Updates 8 different script types with 40+ parameter adjustments
```

**Impact:**
- Adding a new script type requires modifying main.cpp
- Input handlers are tightly coupled to script logic
- Can't test input handling without full hardware
- State machine transitions are implicit, hard to track

---

### 3. **Include Dependency Chain** (HIGH)

**Current structure:**
```
ui.h
  ├─ display.h
  ├─ script_manager.h
  │   ├─ lfo_script.h
  │   ├─ poliquencer_script.h
  │   │   └─ chord_sequencer_script.h
  │   ├─ chord_sequencer_script.h
  │   └─ chord_ranking_engine.h
  │       └─ chord_sequencer_script.h (circular potential)
  └─ chord_ranking_engine.h
      └─ chord_sequencer_script.h

main.cpp includes ui.h → loads 10+ header files
```

**Problem:** Changes to any script class force recompilation of UI and main

**Risk:** One misplaced include creates circular dependency (already fragile with forward declarations)

---

### 4. **Duplicated State Tracking** (HIGH)

**UI maintains shadow copies:**
```cpp
// ui.h - ScriptSlot structure (lines 23-114)
struct ScriptSlot {
    // LFO state
    uint8_t waveType;
    float lfoFrequency;
    uint8_t lastLfoWaveType;      // Duplicate for flicker detection
    float lastLfoFrequency;
    
    // Sequencer state
    int8_t seqStepValues[8];
    uint8_t seqStepDurations[8];
    int8_t lastSeqStepValues[8];  // Duplicate for flicker detection
    uint8_t lastSeqStepDurations[8];
    
    // Chord state
    uint8_t chordRoots[MAX_CHORD_SLOTS];
    uint8_t lastChordRoots[MAX_CHORD_SLOTS];
    // ... 50+ more fields
};

// But scripts also maintain this data:
// - LFOScript::waveType, lfoFrequency, lfoLevel
// - PoliquencerScript::stepValues[8], stepDurations[8], stepGateModes[8]
// - ChordSequencerScript::chords[MAX_CHORD_SLOTS], chordBeats
```

**Why this happened:** UI needed to detect changes for flicker-free rendering, so it copied script state + "last" version

**Problem:** 
- Multiple sources of truth (script owns data, UI owns display cache)
- Risk of desync (user edits value, UI displays old value)
- Hard to understand which one is canonical

**Evidence (Quantization Bug):**
The double-offset bug we just fixed is a result of this confusion:
```cpp
// Poliquencer gets chord root from ChordSequencer
uint8_t rootNote;
chordSequencer->getChord(chordSlot, &rootNote, &chordType);

// But ChordSequencer already provides rootNote in ScaleInfo
chordSequencer->getCurrentScale(&scale);  // scale.rootNote is set!

// Quantization function then uses rootNote AGAIN:
quantizeToScale(volts, rootNote, scale.notes);  // Double offset!
```

---

### 5. **Hand-Coded State Management** (MEDIUM)

**Problem:** No clear pattern for tracking state changes

**Current approach:**
```cpp
// For flicker detection:
scriptSlots[i].lastLfoWaveType = 255;      // Initialize to impossible value
if (lfoWaveType != lastLfoWaveType) {
    // Redraw only if changed
    lastLfoWaveType = lfoWaveType;
}

// But inconsistently applied:
// Some UI elements check this, some redraw every frame
// Some state has "last" tracking, some doesn't
```

**Better approach would be:** Explicit change detection or event system
```cpp
// Could be:
if (ui.hasChanged(slot, UI_PROPERTY_LFO_WAVE_TYPE)) {
    ui.redrawLFOWaveType(slot);
}
```

---

### 6. **Missing Abstractions**

**Input Handling** (scattered across main.cpp):
- No clear input → command mapping
- No queue for input events
- Direct manipulation of UI/script state from input handlers
- Touch, encoder, and buttons mixed with script editing logic

**Suggestion:** Create an `InputDispatcher` that translates hardware events to commands:
```cpp
struct InputCommand {
    enum Type { ENCODER_DELTA, BUTTON_PRESSED, TOUCH } type;
    uint8_t value;
    // route to handler without mixing concerns
};
```

**Script Communication** (tightly coupled):
- UI directly calls script methods
- No validation/contract checking
- Scripts can be in bad state if UI makes wrong calls in wrong order

**Suggestion:** Create explicit script interface contracts

---

### 7. **File Organization Issues** (MEDIUM)

**Current structure:**
```
include/
  - ui.h (344 lines) ← too large
  - script_manager.h (155 lines)
  - poliquencer_script.h (131 lines)
  - chord_sequencer_script.h (167 lines)
  - chord_ranking_engine.h (93 lines)
  - lfo_script.h (66 lines)
  - [no input_handler.h - just input.h with InputHandler class]
  - [no state machine or menu definitions]

src/
  - ui.cpp (3,151 lines) ← monolithic
  - main.cpp (1,034 lines) ← too large
  - script_manager.cpp (849 lines)
  - chord_ranking_engine.cpp (396 lines)
  - [UI rendering code is all in one file, no separation by feature]
```

**Missing layers:**
- No state machine abstraction (AppState enum lives in main.cpp)
- No menu system abstraction (menu logic in main.cpp)
- Rendering code not separated by concern (LFO rendering, Sequencer rendering, etc.)

---

## Severity Classification

| Issue | Severity | Impact | Effort to Fix |
|-------|----------|--------|---------------|
| Monolithic ui.cpp (3,151 lines) | **CRITICAL** | Can't modify UI without risk; hard to debug | 6-8 hours |
| main.cpp too large (1,034 lines) | **CRITICAL** | Adding script type requires main.cpp edit; state unclear | 4-6 hours |
| Duplicated script state in UI | **HIGH** | Risk of desync; causes bugs like quantization issue | 3-4 hours |
| Include chain complexity | **HIGH** | Slow recompiles; fragile to circular deps | 1-2 hours |
| No input abstraction | **MEDIUM** | Hard to add new input; mixing concerns | 3-4 hours |
| Missing script abstractions | **MEDIUM** | Scripts have implicit contracts; error-prone | 2-3 hours |
| File organization | **LOW** | Just organization; doesn't affect function | 1-2 hours |

---

## Root Causes

### Why did this happen?

1. **Rapid Prototyping:** You built features fast, added LFO → Sequencer → Poliquencer → Chord Sequencer incrementally without refactoring between.

2. **UI Complexity Underestimated:** Started simple (one script), then added overlays (carousel, chord list, beat picker) without refactoring base structure.

3. **No Clear Module Boundaries:** Scripts kept growing, UI kept growing, main.cpp became the "glue" that does everything.

4. **State Tracking Band-Aid:** Flicker problem drove you to duplicate state (UI + UI's "last" copy), masking the deeper issue (UI and scripts weren't properly decoupled).

5. **Incremental Bug Fixes:** Each fix added special cases (e.g., quantization double-offset), not addressing the root fragmentation.

---

## Recommended Refactoring Path

### Phase 1: Establish Baseline (1-2 hours)
- [x] Analysis (you are here)
- [ ] Document current feature list
- [ ] Create test script to verify current behavior
- [ ] Create git branch for refactoring

### Phase 2: Decouple UI State from Scripts (6-8 hours)
**Goal:** Remove duplicated state, make UI a pure view layer

**Steps:**
1. Move script parameter editing logic out of main.cpp → new `ScriptEditor` class
2. Replace `ScriptSlot` duplication with proper change detection
3. Simplify UI state → just viewport/selection tracking, not data copies
4. Create clean data-to-UI contract (scripts provide read-only data, UI renders)

**Outcome:** ui.cpp shrinks from 3,151 to ~1,500 lines; main.cpp shrinks from 1,034 to ~300 lines

### Phase 3: Extract Input Handling (3-4 hours)
**Goal:** Separate input processing from business logic

**Steps:**
1. Create `InputDispatcher` to map hardware events → commands
2. Create command handlers for each script type (e.g., `LFOInputHandler`, `PoliquencerInputHandler`)
3. Move main.cpp input handling to separate handlers
4. Centralize state machine in new `ApplicationController` class

**Outcome:** main.cpp becomes pure state machine; input handling is testable

### Phase 4: Reduce Include Chain (1-2 hours)
**Goal:** Decouple modules via forward declarations and interfaces

**Steps:**
1. Move UI-specific data structures out of script headers
2. Create `ScriptInterface` abstract base class with pure virtual getters
3. Replace concrete includes with forward declarations where possible
4. Move enums/constants to separate header files

**Outcome:** Faster compile times; easier to add new script types

### Phase 5: File Reorganization (1-2 hours)
**Goal:** Clearer project structure

**New structure:**
```
include/
  core/
    - config.h
    - types.h (shared enums, constants)
  scripts/
    - script_interface.h (abstract base)
    - lfo_script.h
    - poliquencer_script.h
    - chord_sequencer_script.h
  ui/
    - ui.h (simplified, 200 lines)
    - ui_renderer.h (drawing interface)
  app/
    - input_dispatcher.h
    - application_controller.h
    - state_machine.h
  hardware/
    - display.h
    - input.h
    - dac.h

src/
  core/
    - types.cpp (if needed)
  scripts/
    - [script implementations]
  ui/
    - ui.cpp (simplified)
    - ui_lfo_renderer.cpp (isolated)
    - ui_sequencer_renderer.cpp (isolated)
    - ui_chord_renderer.cpp (isolated)
  app/
    - input_dispatcher.cpp
    - application_controller.cpp
    - main.cpp (100 lines, just setup)
  hardware/
    - [hardware implementations]
```

---

## Key Refactoring Principles to Apply

### 1. **Unidirectional Data Flow**
```
Hardware Input
    ↓
InputDispatcher (maps to commands)
    ↓
ScriptManager (owns data & logic)
    ↓
UI (read-only view of data)
    ↓
Display (renders)
```

Currently: Bidirectional (UI reads/writes script data, input handlers directly modify both)

### 2. **Separation of Concerns**
- **Scripts:** Own data and algorithms (quantization, sequencing logic)
- **UI:** Owns display cache (what's drawn), rendering logic, viewport state
- **Input:** Owns event processing, command creation
- **Main:** Owns state machine, initialization, task orchestration

Currently: All mixed in main.cpp and ui.cpp

### 3. **Single Responsibility for Classes**
- Don't: `UI` class with 150 methods doing rendering, state tracking, parameter editing
- Do: `UI` (viewports/menus) + `UIRenderer` (rendering) + `UIStateCache` (flicker detection)

### 4. **Explicit Dependencies**
- Don't: `#include "script_manager.h"` in ui.h
- Do: Forward declare or use interfaces

---

## How I Can Support You

**I can do all of the refactoring:** Extract UI rendering functions, create new abstractions, move code between files, update all interdependencies, maintain backward compatibility during refactoring.

**You decide:**
1. How much time to invest (Phase 2 only? Phases 2-3? Full refactor?)
2. Architecture preferences (is Phase 3 structure appealing?)
3. Timeline (do it now, or continue feature work first?)

---

## Estimated Effort

| Phase | Time | Breaking Changes? |
|-------|------|-------------------|
| Phase 2: Decouple UI State | 6-8 hrs | Yes, but internal; features unaffected |
| Phase 3: Extract Input | 3-4 hrs | Yes, but clean up; no feature loss |
| Phase 4: Reduce Includes | 1-2 hrs | No |
| Phase 5: File Reorganization | 1-2 hrs | No (just moving code) |
| **TOTAL** | **12-16 hrs** | Significant improvement in maintainability |

---

## Next Steps

1. **Decide scope:** Do you want to refactor now or continue building features?
2. **Priority:** What matters most?
   - Faster iteration on new scripts?
   - Easier debugging of current issues?
   - Better code organization for sharing?
3. **I can:**
   - Start with Phase 2 (biggest impact, highest effort)
   - Start with Phase 3 (cleaner input handling)
   - Do the full refactor in stages
   - Address just the highest-priority issues

Let me know and I'll start executing!

