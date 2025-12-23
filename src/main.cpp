/**
 * Polyphonion - Eurorack Synthesizer Firmware
 * 
 * A Teensy 4.1-based synthesizer in Doepfer Eurorack format with
 * Norns Shield-style graphical GUI. Supports multitasking with
 * 4 concurrent SuperCollider-compatible scripts.
 * 
 * Hardware:
 * - Teensy 4.1
 * - ILI9488 3.5" TFT Display (320x480)
 * - 2 buttons: OK (pin 2), Back (pin 3)
 * - Rotary encoder for scrolling (CLK: pin 4, DT: pin 5, SW: pin 6)
 * 
 * Author: Polyphonion Team
 * License: MIT
 */

#include <Arduino.h>
#include <SPI.h>
#include "config.h"
#include "display.h"
#include "input.h"
#include "ui.h"
#include "script_manager.h"

// Global objects
Display display;
InputHandler input;
UI ui;
ScriptManager scriptManager;

// Application state
enum class AppState {
    WELCOME,
    MAIN_MENU,
    SCRIPT_SELECT,
    SCRIPT_LIBRARY,  // Browse available scripts
    SCRIPT_RUNNING,
    SETTINGS
};

AppState currentState = AppState::WELCOME;
AppState previousState = AppState::MAIN_MENU;
unsigned long welcomeStartTime = 0;
const unsigned long WELCOME_DURATION = 2000; // 2 seconds
bool welcomeScreenDrawn = false;

// Function declarations
void handleWelcomeState();
void handleMainMenuState();
void handleScriptSelectState();
void handleScriptLibraryState();
void handleScriptRunningState();
void handleSettingsState();
void handleTestConsole();

// Helper to change state and track history
void changeState(AppState newState) {
    previousState = currentState;
    currentState = newState;
}

void setup() {
    // Initialize serial for debugging
    Serial.begin(115200);
    delay(500);
    Serial.println("\n=== Polyphonion Initializing ===");
    
    // Initialize hardware
    Serial.println("Initializing display...");
    display.begin();
    Serial.println("Display OK");
    
    Serial.println("Initializing input...");
    input.begin();
    Serial.println("Input OK");
    
    Serial.println("Initializing UI...");
    ui.begin(&display);
    Serial.println("UI OK");
    
    Serial.println("Initializing ScriptManager...");
    scriptManager.begin();
    Serial.println("ScriptManager OK");
    
    // Set scriptManager pointer in UI for per-chord parameter access
    ui.setScriptManager(&scriptManager);
    
    // Start with welcome screen
    welcomeStartTime = millis();
    Serial.println("\n=== Polyphonion Ready ===");
}

void loop() {
    // Update input state
    input.update();

    // Handle serial test console (non-blocking)
    handleTestConsole();
    
    // Handle state machine
    switch (currentState) {
        case AppState::WELCOME:
            handleWelcomeState();
            break;
            
        case AppState::MAIN_MENU:
            handleMainMenuState();
            break;
            
        case AppState::SCRIPT_SELECT:
            handleScriptSelectState();
            break;
        
        case AppState::SCRIPT_LIBRARY:
            handleScriptLibraryState();
            break;
            
        case AppState::SCRIPT_RUNNING:
            handleScriptRunningState();
            break;
            
        case AppState::SETTINGS:
            handleSettingsState();
            break;
    }
    
    // Update running scripts
    scriptManager.update();
    
    // Small delay to prevent excessive looping
    delay(1);
}

void handleWelcomeState() {
    // Draw welcome screen only once
    if (!welcomeScreenDrawn) {
        ui.showWelcomeScreen();
        welcomeScreenDrawn = true;
    }
    
    // Transition to main menu after welcome duration
    if (millis() - welcomeStartTime >= WELCOME_DURATION) {
        currentState = AppState::MAIN_MENU;
        ui.showMainMenu();
    }
    
    // Allow skip with OK button
    if (input.isButtonPressed(BTN_OK)) {
        currentState = AppState::MAIN_MENU;
        ui.showMainMenu();
    }
}

void handleMainMenuState() {
    // Handle scrolling with encoder
    int scrollDelta = input.getEncoderDelta();
    if (scrollDelta != 0) {
        ui.scrollMenu(scrollDelta);
        ui.showMainMenu();  // Partial redraw of changed items
    }
    
    // Handle button presses
    if (input.isButtonPressed(BTN_OK)) {
        int selection = ui.getSelectedMenuItem();
        switch (selection) {
            case 0: // Scripts
                changeState(AppState::SCRIPT_LIBRARY);
                ui.resetMenuTracking();  // Reset for new screen
                ui.setMenuItemCount(scriptManager.getScriptLibraryCount());
                ui.showScriptLibraryScreen(&scriptManager);
                break;
            case 1: // Settings
                changeState(AppState::SETTINGS);
                ui.resetMenuTracking();  // Reset for new screen
                ui.showSettingsScreen();
                break;
            case 2: // About
                ui.showAboutScreen();
                break;
        }
    }
}

void handleScriptSelectState() {
    static unsigned long lastRefresh = 0;
    
    // Active slot is whichever script the UI is currently showing
    uint8_t activeSlot = ui.getSelectedSlot();
    if (activeSlot >= MAX_SCRIPTS) {
        activeSlot = 0;
    }
    
    // Check if touch test is running in the active slot
    bool isTouchTest = (ui.getScriptType(activeSlot) == 3);
    
    // Log all input events for touch test mode
    if (isTouchTest) {
        static bool lastOK = false;
        static bool lastBack = false;
        static bool lastTouch = false;
        
        // Check OK button
        if (input.isButtonPressed(BTN_OK) && !lastOK) {
            Serial.println(">>> OK BUTTON PRESSED");
            lastOK = true;
        } else if (!input.isButtonPressed(BTN_OK)) {
            lastOK = false;
        }
        
        // Check Back button
        if (input.isButtonPressed(BTN_BACK) && !lastBack) {
            Serial.println(">>> BACK BUTTON PRESSED");
            lastBack = true;
        } else if (!input.isButtonPressed(BTN_BACK)) {
            lastBack = false;
        }
        
        // Check touch - show BOTH raw and mapped coordinates for calibration
        if (input.wasTouched()) {
            int16_t tx, ty;
            input.getTouchPoint(&tx, &ty);
            
            // Get raw coordinates directly from touch controller for calibration
            if (input.isTouched()) {
                TS_Point rawPoint = input.getRawTouchPoint();
                Serial.print(">>> RAW TOUCH: x=");
                Serial.print(rawPoint.x);
                Serial.print(" y=");
                Serial.print(rawPoint.y);
                Serial.print(" z=");
                Serial.print(rawPoint.z);
                Serial.print(" | MAPPED: x=");
                Serial.print(tx);
                Serial.print(" y=");
                Serial.println(ty);
            }
            input.clearTouch();
        }
        
        if (input.isTouched() && !lastTouch) {
            Serial.println(">>> TOUCH ACTIVE (held)");
            lastTouch = true;
        } else if (!input.isTouched()) {
            lastTouch = false;
        }
    }
    
    // Update script outputs periodically
    if (millis() - lastRefresh > 50) {  // Refresh every 50ms for smooth animation
        for (int i = 0; i < MAX_SCRIPTS; i++) {
            if (scriptManager.isScriptRunning(i)) {
                const char* output = scriptManager.getScriptOutput(i);
                ui.updateScriptOutput(i, output);
                
                // Update waveform data if it's an LFO
                uint8_t waveType;
                float phase;
                if (scriptManager.getLFOWaveformData(i, &waveType, &phase)) {
                    ui.updateScriptWaveform(i, waveType, phase);
                }
                
                // Update sequencer data if it's a sequencer
                uint8_t currentStep;
                int8_t stepValues[8];
                uint8_t stepDurations[8];
                if (scriptManager.getSequencerData(i, &currentStep, stepValues, stepDurations)) {
                    ui.updateScriptSequencer(i, currentStep, stepValues, stepDurations);
                }
                
                // Update poliquencer sequencer data if it's a poliquencer sequencer
                uint8_t currentBeat;
                uint8_t gateModes[8];
                uint8_t direction;
                bool steamTrigger;
                if (scriptManager.getPoliquencerData(i, &currentStep, &currentBeat, stepValues, stepDurations, gateModes, &direction, &steamTrigger)) {
                    ui.updatePoliquencerSequencer(i, currentStep, currentBeat, stepValues, stepDurations, gateModes, direction, steamTrigger);
                }
                
                // Update chord sequencer data if it's a chord sequencer
                uint8_t chordRoots[MAX_CHORD_SLOTS];
                uint8_t chordTypes[MAX_CHORD_SLOTS];
                uint8_t chordBeats[MAX_CHORD_SLOTS];
                uint8_t currentChordSlot;
                uint8_t beatCounter;
                uint8_t chordCount;
                if (scriptManager.getChordSequencerData(i, chordRoots, chordTypes, chordBeats, &currentChordSlot, &beatCounter, &chordCount)) {
                    ui.updateChordSequencer(i, chordRoots, chordTypes, chordBeats, currentChordSlot, beatCounter, chordCount);
                    
                    // Also update global parameters
                    GlobalParameters globals;
                    if (scriptManager.getChordSequencerGlobals(i, &globals)) {
                        ui.updateChordSequencerGlobals(i, globals);
                    }
                }
            }
        }
        lastRefresh = millis();
        ui.showScriptSelectScreen();  // Refresh display
    }
    
    // Handle encoder input
    int scrollDelta = input.getEncoderDelta();
    if (scrollDelta != 0) {
        // Adjust parameters for the active slot based on script type
        if (scriptManager.isScriptRunning(activeSlot)) {
            uint8_t dummy;
            int8_t dummySteps[8];
            uint8_t dummyDurations[8];
            uint8_t dummyGateModes[8];
            uint8_t dummyDirection;
            bool dummySteam;
            
            // Try LFO first
            uint8_t waveType;
            float phase;
            if (scriptManager.getLFOWaveformData(activeSlot, &waveType, &phase)) {
                uint8_t editParam = ui.getLFOEditParam(activeSlot);
                
                if (editParam == 0) {
                    // Waveform editing
                    ui.adjustLFOWaveform(activeSlot, scrollDelta);
                    scriptManager.setLFOWaveform(activeSlot, ui.getLFOWaveType(activeSlot));
                } else if (editParam == 1) {
                    // Frequency editing
                    ui.adjustLFOFrequency(activeSlot, scrollDelta);
                    scriptManager.setLFOFrequency(activeSlot, ui.getLFOFrequency(activeSlot));
                } else if (editParam == 2) {
                    // Level editing
                    ui.adjustLFOLevel(activeSlot, scrollDelta);
                    scriptManager.setLFOLevel(activeSlot, ui.getLFOLevel(activeSlot));
                }
            }
            // Try regular sequencer
            else if (scriptManager.getSequencerData(activeSlot, &dummy, dummySteps, dummyDurations)) {
                uint8_t editStep = ui.getSequencerEditStep(activeSlot);
                if (ui.isEditingDuration(activeSlot)) {
                    // Editing duration - adjust dial value
                    ui.adjustSequencerStepDuration(activeSlot, scrollDelta);
                    scriptManager.setSequencerStepDuration(activeSlot, editStep, dummyDurations[editStep] + scrollDelta);
                } else {
                    // Editing pitch - adjust slider value
                    ui.adjustSequencerStepValue(activeSlot, scrollDelta);
                    scriptManager.setSequencerStepValue(activeSlot, editStep, dummySteps[editStep] + scrollDelta);
                }
            } 
            // Try poliquencer sequencer
            else if (scriptManager.getPoliquencerData(activeSlot, &dummy, &dummy, dummySteps, dummyDurations, dummyGateModes, &dummyDirection, &dummySteam)) {
                uint8_t editStep = ui.getSequencerEditStep(activeSlot);
                uint8_t editMode = ui.getSequencerEditMode(activeSlot);
                
                if (editMode == 0) {
                    // Mode 0: Editing pitch - adjust lever value
                    ui.adjustSequencerStepValue(activeSlot, scrollDelta);
                    scriptManager.setPoliquencerStepValue(activeSlot, editStep, dummySteps[editStep] + scrollDelta);
                } else if (editMode == 1) {
                    // Mode 1: Editing gate mode - cycle through switch positions
                    // Get current data first
                    scriptManager.getPoliquencerData(activeSlot, &dummy, &dummy, dummySteps, dummyDurations, dummyGateModes, &dummyDirection, &dummySteam);
                    // Calculate new gate mode
                    int8_t newMode = (int8_t)dummyGateModes[editStep] + scrollDelta;
                    if (newMode < 0) newMode = 2;
                    if (newMode > 2) newMode = 0;
                    // Update UI and script manager
                    ui.adjustStepGateMode(activeSlot, scrollDelta);
                    scriptManager.setPoliquencerStepGateMode(activeSlot, editStep, (uint8_t)newMode);
                } else if (editMode == 2) {
                    // Mode 2: Editing duration - adjust crank value
                    ui.adjustSequencerStepDuration(activeSlot, scrollDelta);
                    scriptManager.setPoliquencerStepDuration(activeSlot, editStep, dummyDurations[editStep] + scrollDelta);
                }
            }
            // Try ChordSequencer
            else {
                uint8_t chordRoots[MAX_CHORD_SLOTS];
                uint8_t chordTypes[MAX_CHORD_SLOTS];
                uint8_t chordBeats[MAX_CHORD_SLOTS];
                uint8_t currentChordSlot;
                uint8_t beatCounter;
                uint8_t chordCount;
                if (scriptManager.getChordSequencerData(activeSlot, chordRoots, chordTypes, chordBeats, &currentChordSlot, &beatCounter, &chordCount)) {
                    if (ui.isBeatCountPickerActive(activeSlot)) {
                        // Navigation through 4 chord parameters
                        if (ui.isEditingChordParam(activeSlot)) {
                            // In edit mode - adjust current parameter value
                            ui.adjustChordParam(activeSlot, scrollDelta);
                        } else {
                            // Navigate through the 4 parameters
                            ui.navigateBeatCountPicker(activeSlot, scrollDelta);
                        }
                    } else if (ui.isChordListActive(activeSlot)) {
                        ui.navigateChordList(activeSlot, scrollDelta);
                    } else if (ui.isEditingGlobalParam(activeSlot)) {
                        // In global param edit mode - adjust value
                        ui.adjustGlobalParam(activeSlot, scrollDelta);
                    } else {
                        // Navigation includes: chords, plus box, and 4 global param boxes
                        uint8_t chordCountUi = ui.getChordCount(activeSlot);
                        if (chordCountUi > MAX_CHORD_SLOTS) chordCountUi = MAX_CHORD_SLOTS;
                        
                        uint8_t chordBoxes = chordCountUi;
                        uint8_t plusBoxes = (chordCountUi < MAX_CHORD_SLOTS) ? 1 : 0;
                        uint8_t globalBoxes = 5;  // Key, Degree, Theory, Compactness, Energy
                        uint8_t totalNavigable = chordBoxes + plusBoxes + globalBoxes;
                        
                        // Current selection: chord slot (0..chordCount-1), plus box (chordCount), or global param (via selectedGlobalParam)
                        uint8_t currentIndex = 0;
                        uint8_t selectedChord = ui.getSelectedChordSlot(activeSlot);
                        uint8_t selectedGlobal = ui.getSelectedGlobalParam(activeSlot);
                        
                        // Initialize selection if nothing is selected (only on first interaction)
                        if (selectedChord == 255 && selectedGlobal == 255) {
                            // Default to plus box (or first global if no plus box available)
                            if (plusBoxes > 0) {
                                ui.setSelectedChordSlot(activeSlot, chordBoxes);  // Select plus box (index = chordBoxes)
                                selectedChord = chordBoxes;  // Update local variable
                            } else {
                                ui.setSelectedGlobalParam(activeSlot, 0);  // Select first global param
                                selectedGlobal = 0;  // Update local variable
                            }
                        }
                        
                        // Determine current index based on what's selected
                        if (selectedGlobal != 255) {
                            // Currently on a global param
                            currentIndex = chordBoxes + plusBoxes + selectedGlobal;
                        } else {
                            // Currently on a chord or plus box
                            currentIndex = selectedChord;
                        }
                        
                        // Navigate
                        if (scrollDelta > 0) {
                            currentIndex = (currentIndex + 1) % totalNavigable;
                        } else if (scrollDelta < 0) {
                            currentIndex = (currentIndex == 0) ? (totalNavigable - 1) : (currentIndex - 1);
                        }
                        
                        // Update selection based on new index
                        uint8_t plusBoxIndex = chordBoxes;
                        uint8_t firstGlobalIndex = chordBoxes + plusBoxes;
                        
                        if (currentIndex < chordBoxes) {
                            // Chord selection
                            ui.setSelectedChordSlot(activeSlot, currentIndex);
                            ui.setSelectedGlobalParam(activeSlot, 255);  // Deselect global
                        } else if (currentIndex == plusBoxIndex && plusBoxes > 0) {
                            // Plus box selection
                            ui.setSelectedChordSlot(activeSlot, chordBoxes);
                            ui.setSelectedGlobalParam(activeSlot, 255);  // Deselect global
                        } else {
                            // Global param selection
                            uint8_t globalIdx = currentIndex - firstGlobalIndex;
                            ui.setSelectedChordSlot(activeSlot, 255);  // Deselect chord
                            ui.setSelectedGlobalParam(activeSlot, globalIdx);
                        }
                    }
                }
            }
        }
    }
    
    // Handle button input for ChordSequencer
    if (input.isButtonPressed(BTN_OK) && scriptManager.isScriptRunning(activeSlot)) {
        // Check if ChordSequencer is running
        uint8_t chordRoots[MAX_CHORD_SLOTS];
        uint8_t chordTypes[MAX_CHORD_SLOTS];
        uint8_t chordBeats[MAX_CHORD_SLOTS];
        uint8_t currentChordSlot;
        uint8_t beatCounter;
        uint8_t chordCount;
        if (scriptManager.getChordSequencerData(activeSlot, chordRoots, chordTypes, chordBeats, &currentChordSlot, &beatCounter, &chordCount)) {
            if (ui.isBeatCountPickerActive(activeSlot)) {
                uint8_t selectedParam = ui.getSelectedChordParam(activeSlot);
                
                if (ui.isEditingChordParam(activeSlot)) {
                    // Currently editing a parameter - save and exit edit mode
                    ui.exitChordParamEdit(activeSlot, true);
                } else if (selectedParam == 4) {
                    // "Done" button selected - close the picker
                    ui.finalizeBeatCountPicker(activeSlot);
                } else {
                    // Parameter box selected - enter edit mode
                    ui.enterChordParamEdit(activeSlot);
                }
            } else if (ui.isChordListActive(activeSlot)) {
                uint8_t appliedSlot = ui.selectFromChordList(activeSlot);
                if (appliedSlot < MAX_CHORD_SLOTS) {
                    uint8_t newRoot = ui.getChordRoot(activeSlot, appliedSlot);
                    uint8_t newType = ui.getChordType(activeSlot, appliedSlot);
                    scriptManager.setChordSequencerChord(activeSlot, appliedSlot, newRoot, newType);
                    
                    // After selecting a chord from the library, get its current beat count
                    // If it's a new chord being added, default to 8 beats
                    uint8_t initialBeats = 8;
                    
                    // Check if this slot already had a chord with beats set
                    uint8_t chordRoots[MAX_CHORD_SLOTS];
                    uint8_t chordTypes[MAX_CHORD_SLOTS];
                    uint8_t chordBeats[MAX_CHORD_SLOTS];
                    uint8_t currentChordSlot;
                    uint8_t beatCounter;
                    uint8_t chordCount;
                    if (scriptManager.getChordSequencerData(activeSlot, chordRoots, chordTypes, chordBeats, &currentChordSlot, &beatCounter, &chordCount)) {
                        if (appliedSlot < chordCount) {
                            initialBeats = chordBeats[appliedSlot];
                        }
                    }
                    
                    // Open beat count picker with appropriate default
                    ui.openBeatCountPicker(activeSlot, initialBeats);
                    ui.setChordListTarget(activeSlot, appliedSlot);  // Set target for beat confirmation
                }
            } else if (ui.isEditingGlobalParam(activeSlot)) {
                // Exit global param edit mode and save changes
                uint8_t selectedGlobal = ui.getSelectedGlobalParam(activeSlot);
                ui.exitGlobalParamEdit(activeSlot, true);
                
                // Sync changed values to script manager
                GlobalParameters currentGlobals;
                if (scriptManager.getChordSequencerGlobals(activeSlot, &currentGlobals)) {
                    if (selectedGlobal == 0) {
                        scriptManager.setChordSequencerRoot(activeSlot, (MusicalRoot)ui.getGlobalRoot(activeSlot));
                    } else if (selectedGlobal == 1) {
                        scriptManager.setChordSequencerDegree(activeSlot, (ScaleDegree)ui.getGlobalDegree(activeSlot));
                    } else if (selectedGlobal == 2) {
                        scriptManager.setChordSequencerTheoryMode(activeSlot, (TheoryMode)ui.getGlobalTheoryMode(activeSlot));
                    } else if (selectedGlobal == 3) {
                        scriptManager.setChordSequencerVoiceLeading(activeSlot, ui.getGlobalVoiceLeading(activeSlot));
                    } else if (selectedGlobal == 4) {
                        scriptManager.setChordSequencerEnergy(activeSlot, ui.getGlobalEnergy(activeSlot));
                    }
                }
            } else {
                // Check if a global param is selected
                uint8_t selectedGlobal = ui.getSelectedGlobalParam(activeSlot);
                
                if (selectedGlobal != 255) {
                    // Enter global param edit mode
                    ui.enterGlobalParamEdit(activeSlot);
                } else {
                    // Chord or plus box selected - open chord list to select/change chord
                    uint8_t targetSlot = ui.getSelectedChordSlot(activeSlot);
                    ui.openChordList(activeSlot, targetSlot);
                }
            }
        }
    }
    
    // Handle touch input for Poliquencer
    if (input.wasTouched() && scriptManager.isScriptRunning(activeSlot)) {
        int16_t touchX, touchY;
        input.getTouchPoint(&touchX, &touchY);
        
        uint8_t dummy;
        int8_t dummySteps[8];
        uint8_t dummyDurations[8];
        uint8_t dummyGateModes[8];
        uint8_t dummyDirection;
        bool dummySteam;
        
        // Check if Poliquencer is running
        if (scriptManager.getPoliquencerData(activeSlot, &dummy, &dummy, dummySteps, dummyDurations, dummyGateModes, &dummyDirection, &dummySteam)) {
            // Poliquencer layout: Calculate positions dynamically based on screen size
            // Slot 0 content area: y=40 (title margin), h=140 (SCREEN_HEIGHT/2 - title)
            int16_t slotContentY = 40;
            int16_t slotContentH = (SCREEN_HEIGHT / 2) - 20;
            int16_t switchH = 22;
            int16_t crankH = 55;
            int16_t spacing = 3;
            
            // Calculate Y positions matching ui.cpp layout
            int16_t leverY = slotContentY;
            int16_t crankY = slotContentY + slotContentH - crankH - 3;
            int16_t switchY = crankY - switchH - spacing;
            int16_t leverEndY = switchY;  // Levers end where switches begin
            
            int16_t controlWidth = 37;
            int16_t startX = 3;  // 3px left margin
            
            // Determine which step was touched (0-7)
            if (touchX >= startX && touchX < startX + 8 * controlWidth) {
                uint8_t step = (touchX - startX) / controlWidth;
                if (step > 7) step = 7;
                
                // Determine which control type based on Y position
                if (touchY >= leverY && touchY < leverEndY) {
                    // Levers area - touch upper half to increase, lower half to decrease
                    ui.setSequencerEditStep(activeSlot, step);
                    ui.setSequencerEditMode(activeSlot, 0);  // Lever mode
                    
                    // Get current value
                    scriptManager.getPoliquencerData(activeSlot, &dummy, &dummy, dummySteps, dummyDurations, dummyGateModes, &dummyDirection, &dummySteam);
                    int8_t currentValue = dummySteps[step];
                    
                    // Calculate midpoint of lever area
                    int16_t leverMidY = leverY + (leverEndY - leverY) / 2;
                    
                    if (touchY < leverMidY) {
                        // Upper half - increase pitch
                        int8_t newValue = currentValue + 1;
                        if (newValue > 12) newValue = 12;  // Clamp to +12 semitones
                        scriptManager.setPoliquencerStepValue(activeSlot, step, newValue);
                    } else {
                        // Lower half - decrease pitch
                        int8_t newValue = currentValue - 1;
                        if (newValue < -12) newValue = -12;  // Clamp to -12 semitones
                        scriptManager.setPoliquencerStepValue(activeSlot, step, newValue);
                    }
                } else if (touchY >= switchY && touchY < crankY) {
                    // Switches area
                    ui.setSequencerEditStep(activeSlot, step);
                    ui.setSequencerEditMode(activeSlot, 1);  // Switch mode
                    // Toggle switch cycles: down(1)→center(2)→up(0)→center(2)→down(1)→center(2)...
                    // Gate modes: 0=NORMAL(up), 1=SKIP(down), 2=SLIDE(center)
                    scriptManager.getPoliquencerData(activeSlot, &dummy, &dummy, dummySteps, dummyDurations, dummyGateModes, &dummyDirection, &dummySteam);
                    uint8_t currentMode = dummyGateModes[step];
                    uint8_t toggleDir = ui.getToggleDirection(activeSlot, step);
                    uint8_t newMode;
                    
                    if (currentMode == 2) {
                        // At center - go in current direction
                        if (toggleDir == 0) {
                            newMode = 0;  // center → up
                            // After reaching up, next direction is toward down
                            ui.setToggleDirection(activeSlot, step, 1);
                        } else {
                            newMode = 1;  // center → down
                            // After reaching down, next direction is toward up
                            ui.setToggleDirection(activeSlot, step, 0);
                        }
                    } else {
                        // At up or down - always go back to center
                        newMode = 2;  // up/down → center
                        // Direction stays the same - we'll continue in that direction from center
                    }
                    
                    scriptManager.setPoliquencerStepGateMode(activeSlot, step, newMode);
                } else if (touchY >= crankY && touchY < (slotContentY + slotContentH)) {
                    // Cranks area (wheel + number) - increment duration on any touch
                    ui.setSequencerEditStep(activeSlot, step);
                    ui.setSequencerEditMode(activeSlot, 2);  // Crank mode
                    // Get current duration and increment (1-8, wrapping)
                    scriptManager.getPoliquencerData(activeSlot, &dummy, &dummy, dummySteps, dummyDurations, dummyGateModes, &dummyDirection, &dummySteam);
                    uint8_t newDuration = dummyDurations[step] + 1;
                    if (newDuration > 8) newDuration = 1;  // Wrap to 1
                    scriptManager.setPoliquencerStepDuration(activeSlot, step, newDuration);
                }
            }
        }
        
        input.clearTouch();
    }
    
    // Handle OK button - cycle edit parameter for LFO, advance step for sequencer, or go to library
    if (input.isButtonPressed(BTN_OK)) {
        if (scriptManager.isScriptRunning(activeSlot)) {
            uint8_t dummy;
            int8_t dummySteps[8];
            uint8_t dummyDurations[8];
            uint8_t dummyGateModes[8];
            uint8_t dummyDirection;
            bool dummySteam;
            uint8_t chordRoots[MAX_CHORD_SLOTS];
            uint8_t chordTypes[MAX_CHORD_SLOTS];
            uint8_t chordBeats[MAX_CHORD_SLOTS];
            
            // LFO: cycle edit parameter
            uint8_t waveType;
            float phase;
            if (scriptManager.getLFOWaveformData(activeSlot, &waveType, &phase)) {
                ui.advanceLFOEditParam(activeSlot);
                ui.showScriptSelectScreen();
                return;
            }
            // Regular sequencer: advance edit step
            else if (scriptManager.getSequencerData(activeSlot, &dummy, dummySteps, dummyDurations)) {
                ui.advanceSequencerEditStep(activeSlot);
                ui.showScriptSelectScreen();
                return;
            }
            // Poliquencer sequencer: advance edit step (lever1 -> crank1 -> lever2 -> crank2 ...)
            else if (scriptManager.getPoliquencerData(activeSlot, &dummy, &dummy, dummySteps, dummyDurations, dummyGateModes, &dummyDirection, &dummySteam)) {
                ui.advanceSequencerEditStep(activeSlot);
                ui.showScriptSelectScreen();
                return;
            }
            // ChordSequencer: button is already handled above, don't process further
            else if (scriptManager.getChordSequencerData(activeSlot, chordRoots, chordTypes, chordBeats, &dummy, &dummy, nullptr)) {
                return;  // ChordSequencer button handled already, prevent fallthrough
            }
        }
        
        // No sequencer running - go to script library
        int slot = ui.getSelectedSlot();
        ui.setSelectedSlot(slot);
        currentState = AppState::SCRIPT_LIBRARY;
        ui.resetMenuTracking();
        ui.setMenuItemCount(scriptManager.getScriptLibraryCount());
        ui.showScriptLibraryScreen(&scriptManager);
    }
    
    // Handle Back button - first check for chord sequencer overlays
    if (input.isButtonPressed(BTN_BACK)) {
        // If a script is running in the active slot, check for overlays
        if (scriptManager.isScriptRunning(activeSlot)) {
            if (ui.isEditingGlobalParam(activeSlot)) {
                // Cancel global param editing without saving
                ui.exitGlobalParamEdit(activeSlot, false);
                return;  // Handled - stay in SCRIPT_SELECT
            } else if (ui.isEditingChordParam(activeSlot)) {
                // Exit chord param editing without saving, return to beat count picker
                ui.exitChordParamEdit(activeSlot, false);
                return;  // Handled - stay in SCRIPT_SELECT
            } else if (ui.isBeatCountPickerActive(activeSlot)) {
                // Close beat picker and return to chord list
                ui.confirmBeatCount(activeSlot);
                ui.openChordList(activeSlot, ui.getSelectedChordSlot(activeSlot));
                return;  // Handled - stay in SCRIPT_SELECT
            } else if (ui.isChordListActive(activeSlot)) {
                // Close chord list and return to main chord sequencer view
                ui.selectFromChordList(activeSlot);  // Returns to main view
                return;  // Handled - stay in SCRIPT_SELECT
            }
        }
        
        // No overlay active, or no script running - go to main menu
        changeState(AppState::MAIN_MENU);
        ui.resetMenuTracking();
        ui.showMainMenu();
    }
}

void handleScriptLibraryState() {
    // Handle scrolling through available scripts
    int scrollDelta = input.getEncoderDelta();
    if (scrollDelta != 0) {
        ui.scrollMenu(scrollDelta);
        ui.showScriptLibraryScreen(&scriptManager);  // Partial redraw
    }
    
    // Handle OK button - load selected script
    if (input.isButtonPressed(BTN_OK)) {
        // Find first available empty script slot
        int slot = -1;
        for (int i = 0; i < MAX_SCRIPTS; i++) {
            if (!scriptManager.isScriptRunning(i)) {
                slot = i;
                break;
            }
        }
        
        if (slot < 0) {
            Serial.println("ERROR: No available script slots");
            return;
        }
        
        int libraryIndex = ui.getSelectedMenuItem();
        
        Serial.print("Loading script ");
        Serial.print(libraryIndex);
        Serial.print(" into slot ");
        Serial.println(slot);
        
        if (scriptManager.loadScriptFromLibrary(slot, libraryIndex)) {
            ui.updateScriptStatus(slot, true);
            // Update script info in UI
            const char* scriptName = scriptManager.getScriptName(slot);
            ui.updateScriptInfo(slot, scriptName);
            // Set script type for proper visualization (0=LFO, 2=Poliquencer)
            const ScriptLibraryEntry* entry = scriptManager.getScriptLibraryEntry(libraryIndex);
            ui.setScriptType(slot, entry->scriptType);
            
            Serial.print("Script loaded: ");
            Serial.println(scriptName);
            Serial.print("Script type set to: ");
            Serial.println(entry->scriptType);
            
            // Initialize poliquencer sequencer data if it's a poliquencer
            if (entry->scriptType == 2) {
                int8_t defaultSteps[8] = {0, 4, 7, 12, 10, 7, 5, 2}; // Interesting melodic pattern
                uint8_t defaultDurations[8] = {1, 1, 1, 1, 1, 1, 1, 1}; // 1 beat each
                uint8_t defaultGateModes[8] = {0, 0, 0, 2, 0, 1, 0, 0}; // Normal, normal, normal, slide, normal, skip, normal, normal
                ui.updatePoliquencerSequencer(slot, 0, 0, defaultSteps, defaultDurations, defaultGateModes, 0, false);
                Serial.println("Poliquencer data initialized");
            }
            
            // Go back to script select screen
            changeState(AppState::SCRIPT_SELECT);
            ui.setSelectedScriptSlot(slot);  // Show the newly loaded script
            ui.resetMenuTracking();
            ui.showScriptSelectScreen();
        } else {
            Serial.println("Failed to load script");
        }
    }
    
    // Handle Back button - always go back to main menu
    if (input.isButtonPressed(BTN_BACK)) {
        changeState(AppState::MAIN_MENU);
        ui.resetMenuTracking();
        ui.showMainMenu();
    }
}


void handleScriptRunningState() {
    // Update script displays
    for (int i = 0; i < MAX_SCRIPTS; i++) {
        if (scriptManager.isScriptRunning(i)) {
            ui.updateScriptDisplay(i, scriptManager.getScriptOutput(i));
        }
    }
    
    // Handle Back button - prioritize overlays, then exit script
    if (input.isButtonPressed(BTN_BACK)) {
        if (scriptManager.isScriptRunning(0)) {
            // Check for chord sequencer overlays or edit mode
            if (ui.isEditingGlobalParam(0)) {
                // Cancel global param editing without saving
                ui.exitGlobalParamEdit(0, false);
                return;  // Handled - stay in SCRIPT_RUNNING
            } else if (ui.isEditingChordParam(0)) {
                // Exit chord param editing without saving, return to beat count picker
                ui.exitChordParamEdit(0, false);
                return;  // Handled - stay in SCRIPT_RUNNING
            } else if (ui.isBeatCountPickerActive(0)) {
                // Close beat picker and return to chord list
                ui.confirmBeatCount(0);
                ui.openChordList(0, ui.getSelectedChordSlot(0));
                return;  // Handled - stay in SCRIPT_RUNNING
            } else if (ui.isChordListActive(0)) {
                // Close chord list and return to main chord sequencer view
                ui.selectFromChordList(0);  // Returns to main view
                return;  // Handled - stay in SCRIPT_RUNNING
            }
        }
        
        // No chord sequencer overlay active OR script not running - exit to main menu
        currentState = AppState::MAIN_MENU;
        ui.resetMenuTracking();  // Reset for new screen
        ui.showMainMenu();
    }
}

void handleSettingsState() {
    static bool editMode = false;
    int selectedSetting = ui.getSelectedMenuItem();
    
    // Handle encoder
    int scrollDelta = input.getEncoderDelta();
    if (scrollDelta != 0) {
        if (editMode && selectedSetting == 0) {
            // Editing clock tempo
            float currentTempo = ui.getClockTempo();
            float newTempo = currentTempo + (scrollDelta * 5.0f);  // 5 BPM increments
            ui.setClockTempo(newTempo);
            scriptManager.setGlobalTempo(newTempo);
            ui.resetMenuTracking();  // Force full redraw to show new tempo
            ui.showSettingsScreen();
        } else {
            // Navigate between settings
            ui.scrollMenu(scrollDelta);
            ui.showSettingsScreen();  // Partial redraw
        }
    }
    
    // Handle OK button
    if (input.isButtonPressed(BTN_OK)) {
        // If Input Test is selected (item 2), launch it immediately
        if (selectedSetting == 2) {
            // Load touch test into slot 0
            if (scriptManager.loadScriptFromLibrary(0, 100)) {  // Use 100 as special index for input test
                ui.updateScriptStatus(0, true);
                ui.updateScriptInfo(0, "Input Test");
                ui.setScriptType(0, 3);  // Type 3 = input test
                changeState(AppState::SCRIPT_SELECT);
                ui.resetMenuTracking();
                ui.showScriptSelectScreen();
                editMode = false;  // Reset edit mode
            }
        } else if (selectedSetting == 0) {
            // Toggle edit mode for clock tempo
            editMode = !editMode;
        } else {
            // Toggle other settings
            ui.toggleSettingValue();
        }
    }
    
    // Handle Back button
    if (input.isButtonPressed(BTN_BACK)) {
        changeState(AppState::MAIN_MENU);
        ui.resetMenuTracking();  // Reset for new screen
        ui.showMainMenu();
    }
}

// Lightweight serial test console for automated checks
void handleTestConsole() {
    static char lineBuf[128];
    static bool testConsoleInitialized = false;
    const char* noteNames[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};

    while (Serial && Serial.available()) {
        size_t len = Serial.readBytesUntil('\n', lineBuf, sizeof(lineBuf) - 1);
        lineBuf[len] = '\0';
        String line = String(lineBuf);
        line.trim();
        if (line.length() == 0) continue;
        if (!line.startsWith("TEST")) continue;  // Ignore non-test input

        // Auto-initialize chord sequencer on first TEST command
        if (!testConsoleInitialized) {
            scriptManager.loadScriptFromLibrary(0, 1);  // Load chord sequencer (index 1)
            testConsoleInitialized = true;
            Serial.println("AUTO-INIT: Chord Sequencer loaded to slot 0");
        }

        auto readInt = [&](const char* key, int defaultVal) -> int {
            int pos = line.indexOf(key);
            if (pos < 0) return defaultVal;
            pos += strlen(key);
            int end = line.indexOf(' ', pos);
            String token = (end == -1) ? line.substring(pos) : line.substring(pos, end);
            return (int)token.toInt();
        };

        auto readFloat = [&](const char* key, float defaultVal) -> float {
            int pos = line.indexOf(key);
            if (pos < 0) return defaultVal;
            pos += strlen(key);
            int end = line.indexOf(' ', pos);
            String token = (end == -1) ? line.substring(pos) : line.substring(pos, end);
            return token.toFloat();
        };

        if (line.startsWith("TEST HELP")) {
            Serial.println("TEST commands:");
            Serial.println("  TEST RESET");
            Serial.println("  TEST GLOBAL root=<0-11> degree=<0-6> theory=<0-4> vl=<0-1> energy=<0-1>");
            Serial.println("  TEST CHORD slot=<0-7> root=<0-11> type=<0=maj|1=min> beats=<1-32>");
            Serial.println("  TEST RANK");
            Serial.println("  TEST STATE");
            continue;
        }

        if (line.startsWith("TEST RESET")) {
            scriptManager.resetChordSequencer(0);
            Serial.println("OK RESET");
            continue;
        }

        if (line.startsWith("TEST GLOBAL")) {
            int root = readInt("root=", 0);
            int degree = readInt("degree=", 0);
            int theory = readInt("theory=", 0);
            float vl = readFloat("vl=", 0.5f);
            float energy = readFloat("energy=", 0.5f);

            scriptManager.setChordSequencerRoot(0, (MusicalRoot)root);
            scriptManager.setChordSequencerDegree(0, (ScaleDegree)degree);
            scriptManager.setChordSequencerTheoryMode(0, (TheoryMode)theory);
            scriptManager.setChordSequencerVoiceLeading(0, vl);
            scriptManager.setChordSequencerEnergy(0, energy);

            Serial.print("OK GLOBAL root="); Serial.print(root);
            Serial.print(" degree="); Serial.print(degree);
            Serial.print(" theory="); Serial.print(theory);
            Serial.print(" vl="); Serial.print(vl, 3);
            Serial.print(" energy="); Serial.println(energy, 3);
            continue;
        }

        if (line.startsWith("TEST CHORD")) {
            int slot = readInt("slot=", 0);
            int root = readInt("root=", 0);
            int type = readInt("type=", 0);
            int beats = readInt("beats=", 8);

            scriptManager.setChordSequencerChord(0, (uint8_t)slot, (uint8_t)root, (uint8_t)type);
            scriptManager.setChordSequencerChordBeats(0, (uint8_t)slot, (uint8_t)beats);

            Serial.print("OK CHORD slot="); Serial.print(slot);
            Serial.print(" root="); Serial.print(noteNames[root % 12]);
            Serial.print(" type="); Serial.print(type == 0 ? "maj" : "min");
            Serial.print(" beats="); Serial.println(beats);
            continue;
        }

        if (line.startsWith("TEST RANK")) {
            RankedChord ranked[24];
            uint8_t count = scriptManager.rankChordsForSequencer(0, ranked, 24);
            Serial.print("RANK count="); Serial.println(count);
            for (uint8_t i = 0; i < count && i < 12; i++) {
                Serial.print(i);
                Serial.print(": ");
                Serial.print(noteNames[ranked[i].rootNote % 12]);
                Serial.print(" ");
                Serial.print(ranked[i].type == CHORD_MAJOR ? "maj" : "min");
                Serial.print(" score=");
                Serial.println(ranked[i].totalScore, 4);
            }
            continue;
        }

        if (line.startsWith("TEST STATE")) {
            uint8_t roots[MAX_CHORD_SLOTS];
            uint8_t types[MAX_CHORD_SLOTS];
            uint8_t beats[MAX_CHORD_SLOTS];
            uint8_t currentSlot = 0;
            uint8_t beatCounter = 0;
            uint8_t chordCount = 0;
            if (scriptManager.getChordSequencerData(0, roots, types, beats, &currentSlot, &beatCounter, &chordCount)) {
                Serial.print("STATE chords="); Serial.println(chordCount);
                for (uint8_t i = 0; i < chordCount; i++) {
                    Serial.print("  ");
                    Serial.print(i);
                    Serial.print(": ");
                    Serial.print(noteNames[roots[i] % 12]);
                    Serial.print(" ");
                    Serial.print(types[i] == 0 ? "maj" : "min");
                    Serial.print(" beats=");
                    Serial.println(beats[i]);
                }
            } else {
                Serial.println("STATE ERROR");
            }
            continue;
        }

        Serial.println("ERR UNKNOWN TEST CMD");
    }
}
