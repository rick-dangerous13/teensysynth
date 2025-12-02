/**
 * UI System
 * 
 * Norns-style graphical user interface for Polyphonion
 */

#ifndef UI_H
#define UI_H

#include <Arduino.h>
#include "config.h"
#include "display.h"

// Menu item structure
struct MenuItem {
    const char* label;
    bool enabled;
};

// Script slot structure  
struct ScriptSlot {
    char name[32];
    char path[64];
    char lastPath[64];  // Previous output for change detection
    bool active;
    uint8_t scriptType;  // 0=LFO, 1=Sequencer, etc.
    uint8_t waveType;  // 0=Sine, 1=Triangle, 2=Square, 3=Saw (for LFO)
    float phase;       // Current phase for animation (for LFO)
    // LFO parameters
    float lfoFrequency;  // 0.01 - 100 Hz
    float lfoLevel;      // 0.0 - 5.0 V
    uint8_t lfoEditParam; // 0=waveform, 1=frequency, 2=level
    // LFO state tracking (anti-flicker)
    uint8_t lastLfoWaveType;
    float lastLfoFrequency;
    float lastLfoLevel;
    uint8_t lastLfoEditParam;
    uint8_t seqCurrentStep;  // Current step for sequencer
    int8_t seqStepValues[8]; // Step values for sequencer
    uint8_t seqStepDurations[8]; // Step durations (1-8 beats)
    uint8_t seqEditStep;     // Which step is being edited
    bool seqEditingDuration; // true=editing duration dial, false=editing pitch
    uint8_t seqEditMode;     // 0=pitch, 1=gate mode, 2=duration (for poliquencer)
    // Poliquencer sequencer specific
    uint8_t seqGateModes[8]; // Gate modes: 0=normal, 1=skip, 2=slide
    uint8_t seqDirection;    // Direction: 0=fwd, 1=rev, 2=pendulum, 3=random
    uint8_t seqCurrentBeat;  // Current beat within step (for ratchet animation)
    bool seqSteamTrigger;    // Steam animation trigger
    // Previous state tracking for sequencer (to avoid flickering)
    uint8_t lastSeqCurrentStep;
    int8_t lastSeqStepValues[8];
    uint8_t lastSeqStepDurations[8];
    uint8_t lastSeqGateModes[8];
    uint8_t lastSeqEditStep;
    bool lastSeqEditingDuration;
    uint8_t lastSeqEditMode;
    uint8_t lastSeqDirection;
    uint8_t lastSeqCurrentBeat;
};

class UI {
public:
    UI();
    
    void begin(Display* display);
    
    // Screen rendering
    void showWelcomeScreen();
    void showMainMenu();
    void showScriptSelectScreen();
    void showScriptLibraryScreen();  // Browse available scripts
    void showScriptRunningScreen();
    void showSettingsScreen();
    void showAboutScreen();
    
    // Menu navigation
    void scrollMenu(int16_t delta);
    void resetMenuTracking();  // Call when switching screens
    void setMenuItemCount(int16_t count) { menuItemCount = count; }
    int16_t getSelectedMenuItem();
    int16_t getSelectedSlot();
    void setSelectedSlot(int16_t slot) { selectedScriptSlot = slot; }
    const char* getSelectedScriptPath();
    
    // Script display updates
    void updateScriptStatus(uint8_t slot, bool running);
    void updateScriptInfo(uint8_t slot, const char* name);
    void updateScriptOutput(uint8_t slot, const char* output);  // Update output text
    void updateScriptWaveform(uint8_t slot, uint8_t waveType, float phase);  // Update waveform display
    void updateScriptSequencer(uint8_t slot, uint8_t currentStep, int8_t stepValues[8], uint8_t stepDurations[8]);  // Update sequencer display
    void updateScriptDisplay(uint8_t slot, const char* output);
    void setScriptType(uint8_t slot, uint8_t type);  // Set script type for proper visualization
    uint8_t getScriptType(uint8_t slot) const { return (slot < MAX_SCRIPTS) ? scriptSlots[slot].scriptType : 0; }
    
    // Settings control
    void toggleSettingValue();
    
    // Global clock
    float getClockTempo() const { return clockTempo; }
    void setClockTempo(float bpm);
    
    // Multitasking mode
    bool getMultitaskingMode() const { return multitaskingMode; }
    void toggleMultitaskingMode() { multitaskingMode = !multitaskingMode; }
    
    // Sequencer editing
    uint8_t getSequencerEditStep(uint8_t slot) const { return (slot < MAX_SCRIPTS) ? scriptSlots[slot].seqEditStep : 0; }
    bool isEditingDuration(uint8_t slot) const { return (slot < MAX_SCRIPTS) ? scriptSlots[slot].seqEditingDuration : false; }
    uint8_t getSequencerEditMode(uint8_t slot) const { return (slot < MAX_SCRIPTS) ? scriptSlots[slot].seqEditMode : 0; }
    void setSequencerEditStep(uint8_t slot, uint8_t step);
    void setSequencerEditMode(uint8_t slot, uint8_t mode);
    void advanceSequencerEditStep(uint8_t slot);
    void toggleSequencerEditMode(uint8_t slot);  // Toggle between pitch and duration editing
    void adjustSequencerStepValue(uint8_t slot, int8_t delta);
    void adjustSequencerStepDuration(uint8_t slot, int8_t delta);
    
    // LFO editing
    void advanceLFOEditParam(uint8_t slot);
    void adjustLFOWaveform(uint8_t slot, int delta);
    void adjustLFOFrequency(uint8_t slot, int delta);
    void adjustLFOLevel(uint8_t slot, int delta);
    uint8_t getLFOEditParam(uint8_t slot) const { return (slot < MAX_SCRIPTS) ? scriptSlots[slot].lfoEditParam : 0; }
    uint8_t getLFOWaveType(uint8_t slot) const { return (slot < MAX_SCRIPTS) ? scriptSlots[slot].waveType : 0; }
    float getLFOFrequency(uint8_t slot) const { return (slot < MAX_SCRIPTS) ? scriptSlots[slot].lfoFrequency : 1.0f; }
    float getLFOLevel(uint8_t slot) const { return (slot < MAX_SCRIPTS) ? scriptSlots[slot].lfoLevel : 5.0f; }
    
    // Poliquencer sequencer
    void updatePoliquencerSequencer(uint8_t slot, uint8_t currentStep, uint8_t currentBeat, int8_t stepValues[8], uint8_t stepDurations[8], uint8_t gateModes[8], uint8_t direction, bool steamTrigger);
    void toggleStepGateMode(uint8_t slot);
    void adjustStepGateMode(uint8_t slot, int8_t delta);
    void cycleDirection(uint8_t slot);
    
private:
    Display* display;
    
    // Global settings
    float clockTempo;
    bool multitaskingMode;  // true=4 quadrants, false=full screen
    
    // Menu state
    int16_t menuSelection;
    int16_t lastMenuSelection;  // Track previous selection for partial updates
    int16_t menuItemCount;
    int16_t scrollOffset;
    int16_t selectedScriptSlot;  // Which slot we're loading a script into
    
    // Available menu items
    static const int MAX_MENU_ITEMS = 10;
    MenuItem menuItems[MAX_MENU_ITEMS];
    
    // Script slots for multitasking
    ScriptSlot scriptSlots[MAX_SCRIPTS];
    
    // Helper drawing methods
    void drawMenuItem(int16_t y, const char* label, bool selected);
    void drawScriptSlot(uint8_t slot, bool selected);
    void drawSequencerSliders(uint8_t slot, int16_t x, int16_t y, int16_t w, int16_t h);
    void drawSequencerDials(uint8_t slot, int16_t x, int16_t y, int16_t w, int16_t h);
    void drawPoliquencerSequencer(uint8_t slot, int16_t x, int16_t y, int16_t w, int16_t h);
    void drawHeader(const char* title);
    void drawFooter(const char* leftLabel, const char* rightLabel);
};

#endif // UI_H
