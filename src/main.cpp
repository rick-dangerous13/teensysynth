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
unsigned long welcomeStartTime = 0;
const unsigned long WELCOME_DURATION = 2000; // 2 seconds

// Function declarations
void handleWelcomeState();
void handleMainMenuState();
void handleScriptSelectState();
void handleScriptLibraryState();
void handleScriptRunningState();
void handleSettingsState();

void setup() {
    // Initialize serial for debugging
    Serial.begin(115200);
    delay(100);
    Serial.println("Polyphonion Initializing...");
    
    // Initialize hardware
    display.begin();
    input.begin();
    ui.begin(&display);
    scriptManager.begin();
    
    // Show welcome screen
    currentState = AppState::WELCOME;
    welcomeStartTime = millis();
    ui.showWelcomeScreen();
    
    Serial.println("Polyphonion Ready!");
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
}

void handleWelcomeState() {
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
                currentState = AppState::SCRIPT_LIBRARY;
                ui.resetMenuTracking();  // Reset for new screen
                ui.setMenuItemCount(scriptManager.getScriptLibraryCount());
                ui.showScriptLibraryScreen();
                break;
            case 1: // Settings
                currentState = AppState::SETTINGS;
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
        }
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
        }
        
        // No sequencer running - go to script library
        int slot = ui.getSelectedSlot();
        ui.setSelectedSlot(slot);
        currentState = AppState::SCRIPT_LIBRARY;
        ui.resetMenuTracking();
        ui.setMenuItemCount(scriptManager.getScriptLibraryCount());
        ui.showScriptLibraryScreen();
    }
    
    // Handle Back button
    if (input.isButtonPressed(BTN_BACK)) {
        // Go to script library to change script
        currentState = AppState::SCRIPT_LIBRARY;
        ui.resetMenuTracking();
        ui.setMenuItemCount(scriptManager.getScriptLibraryCount());
        ui.showScriptLibraryScreen();
    }
}

void handleScriptLibraryState() {
    // Handle scrolling through available scripts
    int scrollDelta = input.getEncoderDelta();
    if (scrollDelta != 0) {
        ui.scrollMenu(scrollDelta);
        ui.showScriptLibraryScreen();  // Partial redraw
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
            currentState = AppState::SCRIPT_SELECT;
            ui.resetMenuTracking();
            ui.showScriptSelectScreen();
        } else {
            Serial.println("Failed to load script");
        }
    }
    
    // Handle Back button - cancel and go back
    if (input.isButtonPressed(BTN_BACK)) {
        if (scriptManager.isScriptRunning(0)) {
            // If script running, go back to viewing it
            currentState = AppState::SCRIPT_SELECT;
            ui.resetMenuTracking();
            ui.showScriptSelectScreen();
        } else {
            // No script running, go back to main menu
            currentState = AppState::MAIN_MENU;
            ui.resetMenuTracking();
            ui.showMainMenu();
        }
    }
}


void handleScriptRunningState() {
    // Update script displays
    for (int i = 0; i < MAX_SCRIPTS; i++) {
        if (scriptManager.isScriptRunning(i)) {
            ui.updateScriptDisplay(i, scriptManager.getScriptOutput(i));
        }
    }
    
    // Handle Back button
    if (input.isButtonPressed(BTN_BACK)) {
        currentState = AppState::MAIN_MENU;
        ui.resetMenuTracking();  // Reset for new screen
        ui.showMainMenu();
    }
}

void handleSettingsState() {
    int selectedSetting = ui.getSelectedMenuItem();
    
    // Handle encoder
    int scrollDelta = input.getEncoderDelta();
    if (scrollDelta != 0) {
        // If clock is selected (item 0), adjust tempo
        if (selectedSetting == 0) {
            float currentTempo = ui.getClockTempo();
            float newTempo = currentTempo + (scrollDelta * 5.0f);  // 5 BPM increments
            ui.setClockTempo(newTempo);
            scriptManager.setGlobalTempo(newTempo);
            ui.showSettingsScreen();  // Refresh display
        } else {
            // Navigate between settings for other items
            ui.scrollMenu(scrollDelta);
            ui.showSettingsScreen();  // Partial redraw
        }
    }
    
    // Handle OK button
    if (input.isButtonPressed(BTN_OK)) {
        ui.toggleSettingValue();
    }
    
    // Handle Back button
    if (input.isButtonPressed(BTN_BACK)) {
        currentState = AppState::MAIN_MENU;
        ui.resetMenuTracking();  // Reset for new screen
        ui.showMainMenu();
    }
}
