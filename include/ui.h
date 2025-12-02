/**
 * UI System
 * 
 * Norns-style graphical user interface for TeensySynth
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
    uint8_t seqCurrentStep;  // Current step for sequencer
    int8_t seqStepValues[8]; // Step values for sequencer
    uint8_t seqStepDurations[8]; // Step durations (1-8 beats)
    uint8_t seqEditStep;     // Which step is being edited
    bool seqEditingDuration; // true=editing duration dial, false=editing pitch
    // Previous state tracking for sequencer (to avoid flickering)
    uint8_t lastSeqCurrentStep;
    int8_t lastSeqStepValues[8];
    uint8_t lastSeqStepDurations[8];
    uint8_t lastSeqEditStep;
    bool lastSeqEditingDuration;
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
    void advanceSequencerEditStep(uint8_t slot);
    void toggleSequencerEditMode(uint8_t slot);  // Toggle between pitch and duration editing
    void adjustSequencerStepValue(uint8_t slot, int8_t delta);
    void adjustSequencerStepDuration(uint8_t slot, int8_t delta);
    
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
    void drawHeader(const char* title);
    void drawFooter(const char* leftLabel, const char* rightLabel);
};

#endif // UI_H
