/**
 * Polyphonion - Eurorack Synthesizer Firmware
 * 
 * A Teensy 4.1-based synthesizer in Doepfer Eurorack format with
 * Norns Shield-style graphical GUI. Supports multitasking with
 * 4 concurrent SuperCollider-compatible scripts.
 * 
 * Hardware:
 * - Teensy 4.1
 * - ILI9341 2.8" TFT Display (320x240)
 * - 2 buttons: OK (pin 2), Back (pin 3)
 * - Rotary encoder for scrolling (CLK: pin 4, DT: pin 5, SW: pin 6)
 * 
 * Author: Polyphonion Team
 * License: MIT
 */

#include <Arduino.h>
#include <ILI9341_t3.h>
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
    
    // Check if touch test is running in slot 0
    bool isTouchTest = (ui.getScriptType(0) == 3);
    
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
        // If slot 0 is active, adjust parameters based on script type
        if (scriptManager.isScriptRunning(0)) {
            uint8_t dummy;
            int8_t dummySteps[8];
            uint8_t dummyDurations[8];
            uint8_t dummyGateModes[8];
            uint8_t dummyDirection;
            bool dummySteam;
            
            // Try LFO first
            uint8_t waveType;
            float phase;
            if (scriptManager.getLFOWaveformData(0, &waveType, &phase)) {
                uint8_t editParam = ui.getLFOEditParam(0);
                
                if (editParam == 0) {
                    // Waveform editing
                    ui.adjustLFOWaveform(0, scrollDelta);
                    scriptManager.setLFOWaveform(0, ui.getLFOWaveType(0));
                } else if (editParam == 1) {
                    // Frequency editing
                    ui.adjustLFOFrequency(0, scrollDelta);
                    scriptManager.setLFOFrequency(0, ui.getLFOFrequency(0));
                } else if (editParam == 2) {
                    // Level editing
                    ui.adjustLFOLevel(0, scrollDelta);
                    scriptManager.setLFOLevel(0, ui.getLFOLevel(0));
                }
            }
            // Try regular sequencer
            else if (scriptManager.getSequencerData(0, &dummy, dummySteps, dummyDurations)) {
                uint8_t editStep = ui.getSequencerEditStep(0);
                if (ui.isEditingDuration(0)) {
                    // Editing duration - adjust dial value
                    ui.adjustSequencerStepDuration(0, scrollDelta);
                    scriptManager.setSequencerStepDuration(0, editStep, dummyDurations[editStep] + scrollDelta);
                } else {
                    // Editing pitch - adjust slider value
                    ui.adjustSequencerStepValue(0, scrollDelta);
                    scriptManager.setSequencerStepValue(0, editStep, dummySteps[editStep] + scrollDelta);
                }
            } 
            // Try poliquencer sequencer
            else if (scriptManager.getPoliquencerData(0, &dummy, &dummy, dummySteps, dummyDurations, dummyGateModes, &dummyDirection, &dummySteam)) {
                uint8_t editStep = ui.getSequencerEditStep(0);
                uint8_t editMode = ui.getSequencerEditMode(0);
                
                if (editMode == 0) {
                    // Mode 0: Editing pitch - adjust lever value
                    ui.adjustSequencerStepValue(0, scrollDelta);
                    scriptManager.setPoliquencerStepValue(0, editStep, dummySteps[editStep] + scrollDelta);
                } else if (editMode == 1) {
                    // Mode 1: Editing gate mode - cycle through switch positions
                    // Get current data first
                    scriptManager.getPoliquencerData(0, &dummy, &dummy, dummySteps, dummyDurations, dummyGateModes, &dummyDirection, &dummySteam);
                    // Calculate new gate mode
                    int8_t newMode = (int8_t)dummyGateModes[editStep] + scrollDelta;
                    if (newMode < 0) newMode = 2;
                    if (newMode > 2) newMode = 0;
                    // Update UI and script manager
                    ui.adjustStepGateMode(0, scrollDelta);
                    scriptManager.setPoliquencerStepGateMode(0, editStep, (uint8_t)newMode);
                } else if (editMode == 2) {
                    // Mode 2: Editing duration - adjust crank value
                    ui.adjustSequencerStepDuration(0, scrollDelta);
                    scriptManager.setPoliquencerStepDuration(0, editStep, dummyDurations[editStep] + scrollDelta);
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
                if (scriptManager.getChordSequencerData(0, chordRoots, chordTypes, chordBeats, &currentChordSlot, &beatCounter, &chordCount)) {
                    if (ui.isBeatCountPickerActive(0)) {
                        // Navigation through 4 chord parameters
                        if (ui.isEditingChordParam(0)) {
                            // In edit mode - adjust current parameter value
                            ui.adjustChordParam(0, scrollDelta);
                        } else {
                            // Navigate through the 4 parameters
                            ui.navigateBeatCountPicker(0, scrollDelta);
                        }
                    } else if (ui.isChordListActive(0)) {
                        ui.navigateChordList(0, scrollDelta);
                    } else if (ui.isEditingGlobalParam(0)) {
                        // In global param edit mode - adjust value
                        ui.adjustGlobalParam(0, scrollDelta);
                    } else {
                        // Navigation includes: chords, plus box, and 4 global param boxes
                        uint8_t chordCountUi = ui.getChordCount(0);
                        if (chordCountUi > MAX_CHORD_SLOTS) chordCountUi = MAX_CHORD_SLOTS;
                        
                        uint8_t chordBoxes = chordCountUi;
                        uint8_t plusBoxes = (chordCountUi < MAX_CHORD_SLOTS) ? 1 : 0;
                        uint8_t globalBoxes = 4;
                        uint8_t totalNavigable = chordBoxes + plusBoxes + globalBoxes;
                        
                        // Current selection: chord slot (0..chordCount-1), plus box (chordCount), or global param (via selectedGlobalParam)
                        uint8_t currentIndex = 0;
                        uint8_t selectedChord = ui.getSelectedChordSlot(0);
                        uint8_t selectedGlobal = ui.getSelectedGlobalParam(0);
                        
                        // Initialize selection if nothing is selected (only on first interaction)
                        if (selectedChord == 255 && selectedGlobal == 255) {
                            // Default to plus box (or first global if no plus box available)
                            if (plusBoxes > 0) {
                                ui.setSelectedChordSlot(0, chordBoxes);  // Select plus box (index = chordBoxes)
                                selectedChord = chordBoxes;  // Update local variable
                            } else {
                                ui.setSelectedGlobalParam(0, 0);  // Select first global param
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
                            ui.setSelectedChordSlot(0, currentIndex);
                            ui.setSelectedGlobalParam(0, 255);  // Deselect global
                        } else if (currentIndex == plusBoxIndex && plusBoxes > 0) {
                            // Plus box selection
                            ui.setSelectedChordSlot(0, chordBoxes);
                            ui.setSelectedGlobalParam(0, 255);  // Deselect global
                        } else {
                            // Global param selection
                            uint8_t globalIdx = currentIndex - firstGlobalIndex;
                            ui.setSelectedChordSlot(0, 255);  // Deselect chord
                            ui.setSelectedGlobalParam(0, globalIdx);
                        }
                    }
                }
            }
        }
    }
    
    // Handle button input for ChordSequencer
    if (input.isButtonPressed(BTN_OK) && scriptManager.isScriptRunning(0)) {
        uint8_t dummy;
        int8_t dummySteps[8];
        uint8_t dummyDurations[8];
        uint8_t dummyGateModes[8];
        uint8_t dummyDirection;
        bool dummySteam;
        
        // Check if ChordSequencer is running
        uint8_t chordRoots[MAX_CHORD_SLOTS];
        uint8_t chordTypes[MAX_CHORD_SLOTS];
        uint8_t chordBeats[MAX_CHORD_SLOTS];
        uint8_t currentChordSlot;
        uint8_t beatCounter;
        uint8_t chordCount;
        if (scriptManager.getChordSequencerData(0, chordRoots, chordTypes, chordBeats, &currentChordSlot, &beatCounter, &chordCount)) {
            if (ui.isBeatCountPickerActive(0)) {
                if (ui.isEditingChordParam(0)) {
                    // Currently editing - save and exit edit mode
                    ui.exitChordParamEdit(0, true);
                } else {
                    // Not editing - enter edit mode for the selected parameter
                    ui.enterChordParamEdit(0);
                }
            } else if (ui.isChordListActive(0)) {
                uint8_t appliedSlot = ui.selectFromChordList(0);
                if (appliedSlot < MAX_CHORD_SLOTS) {
                    uint8_t newRoot = ui.getChordRoot(0, appliedSlot);
                    uint8_t newType = ui.getChordType(0, appliedSlot);
                    scriptManager.setChordSequencerChord(0, appliedSlot, newRoot, newType);
                    
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
                    if (scriptManager.getChordSequencerData(0, chordRoots, chordTypes, chordBeats, &currentChordSlot, &beatCounter, &chordCount)) {
                        if (appliedSlot < chordCount) {
                            initialBeats = chordBeats[appliedSlot];
                        }
                    }
                    
                    // Open beat count picker with appropriate default
                    ui.openBeatCountPicker(0, initialBeats);
                    ui.setChordListTarget(0, appliedSlot);  // Set target for beat confirmation
                }
            } else if (ui.isEditingGlobalParam(0)) {
                // Exit global param edit mode and save changes
                uint8_t selectedGlobal = ui.getSelectedGlobalParam(0);
                ui.exitGlobalParamEdit(0, true);
                
                // Sync changed values to script manager
                GlobalParameters currentGlobals;
                if (scriptManager.getChordSequencerGlobals(0, &currentGlobals)) {
                    if (selectedGlobal == 0) {
                        scriptManager.setChordSequencerKey(0, (MusicalKey)ui.getGlobalKey(0));
                    } else if (selectedGlobal == 1) {
                        scriptManager.setChordSequencerTheoryMode(0, (TheoryMode)ui.getGlobalTheoryMode(0));
                    } else if (selectedGlobal == 2) {
                        scriptManager.setChordSequencerVoiceLeading(0, ui.getGlobalVoiceLeading(0));
                    } else if (selectedGlobal == 3) {
                        scriptManager.setChordSequencerEnergy(0, ui.getGlobalEnergy(0));
                    }
                }
            } else {
                // Check if a global param is selected
                uint8_t selectedGlobal = ui.getSelectedGlobalParam(0);
                
                if (selectedGlobal != 255) {
                    // Enter global param edit mode
                    ui.enterGlobalParamEdit(0);
                } else {
                    // Chord or plus box selected - open chord list to select/change chord
                    uint8_t targetSlot = ui.getSelectedChordSlot(0);
                    ui.openChordList(0, targetSlot);
                }
            }
        }
    }
    
    // Handle touch input for Poliquencer
    if (input.wasTouched() && scriptManager.isScriptRunning(0)) {
        int16_t touchX, touchY;
        input.getTouchPoint(&touchX, &touchY);
        
        uint8_t dummy;
        int8_t dummySteps[8];
        uint8_t dummyDurations[8];
        uint8_t dummyGateModes[8];
        uint8_t dummyDirection;
        bool dummySteam;
        
        // Check if Poliquencer is running
        if (scriptManager.getPoliquencerData(0, &dummy, &dummy, dummySteps, dummyDurations, dummyGateModes, &dummyDirection, &dummySteam)) {
            // Poliquencer layout with 3px margins: levers at top, switches middle, cranks bottom
            // Updated: leverWidth = (320 - 6) / 8 - 2 = 37.25px per control
            int16_t controlWidth = 37;
            int16_t startX = 3;  // 3px left margin
            
            // Determine which step was touched (0-7)
            if (touchX >= startX && touchX < startX + 8 * controlWidth) {
                uint8_t step = (touchX - startX) / controlWidth;
                if (step > 7) step = 7;
                
                // Actual layout: leverY=30, leverH=115 (30-145), switchY=150, switchH=30 (150-180), crankY=185 (185-240)
                // Determine which control type based on Y position
                if (touchY >= 30 && touchY < 145) {
                    // Levers area - touch upper half to increase, lower half to decrease
                    ui.setSequencerEditStep(0, step);
                    ui.setSequencerEditMode(0, 0);  // Lever mode
                    
                    // Get current value
                    scriptManager.getPoliquencerData(0, &dummy, &dummy, dummySteps, dummyDurations, dummyGateModes, &dummyDirection, &dummySteam);
                    int8_t currentValue = dummySteps[step];
                    
                    // Calculate midpoint of lever area: 30 + 115/2 = 87
                    int16_t leverMidY = 87;
                    
                    if (touchY < leverMidY) {
                        // Upper half - increase pitch
                        int8_t newValue = currentValue + 1;
                        if (newValue > 12) newValue = 12;  // Clamp to +12 semitones
                        scriptManager.setPoliquencerStepValue(0, step, newValue);
                    } else {
                        // Lower half - decrease pitch
                        int8_t newValue = currentValue - 1;
                        if (newValue < -12) newValue = -12;  // Clamp to -12 semitones
                        scriptManager.setPoliquencerStepValue(0, step, newValue);
                    }
                } else if (touchY >= 145 && touchY < 185) {
                    // Switches area (expanded: actual is 150-180, we expand to 145-185 for easier touch)
                    ui.setSequencerEditStep(0, step);
                    ui.setSequencerEditMode(0, 1);  // Switch mode
                    // Toggle switch cycles: down(1)→center(2)→up(0)→center(2)→down(1)→center(2)...
                    // Gate modes: 0=NORMAL(up), 1=SKIP(down), 2=SLIDE(center)
                    scriptManager.getPoliquencerData(0, &dummy, &dummy, dummySteps, dummyDurations, dummyGateModes, &dummyDirection, &dummySteam);
                    uint8_t currentMode = dummyGateModes[step];
                    uint8_t toggleDir = ui.getToggleDirection(0, step);
                    uint8_t newMode;
                    
                    if (currentMode == 2) {
                        // At center - go in current direction
                        if (toggleDir == 0) {
                            newMode = 0;  // center → up
                            // After reaching up, next direction is toward down
                            ui.setToggleDirection(0, step, 1);
                        } else {
                            newMode = 1;  // center → down
                            // After reaching down, next direction is toward up
                            ui.setToggleDirection(0, step, 0);
                        }
                    } else {
                        // At up or down - always go back to center
                        newMode = 2;  // up/down → center
                        // Direction stays the same - we'll continue in that direction from center
                    }
                    
                    scriptManager.setPoliquencerStepGateMode(0, step, newMode);
                } else if (touchY >= 185 && touchY < 240) {
                    // Cranks area (wheel + number) - increment duration on any touch
                    ui.setSequencerEditStep(0, step);
                    ui.setSequencerEditMode(0, 2);  // Crank mode
                    // Get current duration and increment (1-8, wrapping)
                    scriptManager.getPoliquencerData(0, &dummy, &dummy, dummySteps, dummyDurations, dummyGateModes, &dummyDirection, &dummySteam);
                    uint8_t newDuration = dummyDurations[step] + 1;
                    if (newDuration > 8) newDuration = 1;  // Wrap to 1
                    scriptManager.setPoliquencerStepDuration(0, step, newDuration);
                }
            }
        }
        
        input.clearTouch();
    }
    
    // Handle OK button - cycle edit parameter for LFO, advance step for sequencer, or go to library
    if (input.isButtonPressed(BTN_OK)) {
        if (scriptManager.isScriptRunning(0)) {
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
            if (scriptManager.getLFOWaveformData(0, &waveType, &phase)) {
                ui.advanceLFOEditParam(0);
                ui.showScriptSelectScreen();
                return;
            }
            // Regular sequencer: advance edit step
            else if (scriptManager.getSequencerData(0, &dummy, dummySteps, dummyDurations)) {
                ui.advanceSequencerEditStep(0);
                ui.showScriptSelectScreen();
                return;
            }
            // Poliquencer sequencer: advance edit step (lever1 -> crank1 -> lever2 -> crank2 ...)
            else if (scriptManager.getPoliquencerData(0, &dummy, &dummy, dummySteps, dummyDurations, dummyGateModes, &dummyDirection, &dummySteam)) {
                ui.advanceSequencerEditStep(0);
                ui.showScriptSelectScreen();
                return;
            }
            // ChordSequencer: button is already handled above, don't process further
            else if (scriptManager.getChordSequencerData(0, chordRoots, chordTypes, chordBeats, &dummy, &dummy, nullptr)) {
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
    
    // Handle Back button - go back to where we came from
    if (input.isButtonPressed(BTN_BACK)) {
        AppState destination = previousState;
        currentState = destination;
        // When going back to script library, set previous to main menu for next back press
        if (destination == AppState::SCRIPT_LIBRARY) {
            previousState = AppState::MAIN_MENU;
        } else if (destination == AppState::SETTINGS) {
            previousState = AppState::MAIN_MENU;
        }
        ui.resetMenuTracking();
        
        // Show appropriate screen based on where we're going
        if (destination == AppState::MAIN_MENU) {
            ui.showMainMenu();
        } else if (destination == AppState::SETTINGS) {
            ui.showSettingsScreen();
        } else if (destination == AppState::SCRIPT_LIBRARY) {
            ui.setMenuItemCount(scriptManager.getScriptLibraryCount());
            ui.showScriptLibraryScreen(&scriptManager);
        }
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
        int slot = 0;  // Always use slot 0 (only slot available)
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
    
    // Handle Back button for ChordSequencer
    if (input.isButtonPressed(BTN_BACK) && scriptManager.isScriptRunning(0)) {
        // Check for chord sequencer overlays or edit mode
        if (ui.isEditingGlobalParam(0)) {
            // Cancel global param editing without saving
            ui.exitGlobalParamEdit(0, false);
        } else if (ui.isEditingChordParam(0)) {
            // Exit chord param editing without saving and close beat count picker
            ui.exitChordParamEdit(0, false);
            ui.confirmBeatCount(0);  // Close picker
        } else if (ui.isBeatCountPickerActive(0)) {
            // Close beat picker without saving
            ui.confirmBeatCount(0);  // This closes the picker
        } else if (ui.isChordListActive(0)) {
            // Close chord list without selecting
            ui.selectFromChordList(0);  // Returns to main view
        } else {
            // No overlay active - go back to main menu
            currentState = AppState::MAIN_MENU;
            ui.resetMenuTracking();
            ui.showMainMenu();
        }
        return;  // Handled
    }
    
    // Handle Back button - always go back to main menu
    if (input.isButtonPressed(BTN_BACK)) {
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
