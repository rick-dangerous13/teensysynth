/**
 * TeensySynth - Eurorack Synthesizer Firmware
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
 * Author: TeensySynth Team
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
    Serial.println("TeensySynth Initializing...");
    
    // Initialize hardware
    display.begin();
    input.begin();
    ui.begin(&display);
    scriptManager.begin();
    
    // Show welcome screen
    currentState = AppState::WELCOME;
    welcomeStartTime = millis();
    ui.showWelcomeScreen();
    
    Serial.println("TeensySynth Ready!");
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
                currentState = AppState::SCRIPT_SELECT;
                ui.resetMenuTracking();  // Reset for new screen
                ui.showScriptSelectScreen();
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
                if (scriptManager.getSequencerData(i, &currentStep, stepValues)) {
                    ui.updateScriptSequencer(i, currentStep, stepValues);
                }
            }
        }
        lastRefresh = millis();
        ui.showScriptSelectScreen();  // Refresh display
    }
    
    // Handle encoder input
    int scrollDelta = input.getEncoderDelta();
    if (scrollDelta != 0) {
        // If slot 0 is active and is a sequencer, adjust step value
        if (scriptManager.isScriptRunning(0)) {
            uint8_t dummy;
            int8_t dummySteps[8];
            if (scriptManager.getSequencerData(0, &dummy, dummySteps)) {
                // Sequencer is running - adjust current edit step value
                ui.adjustSequencerStepValue(0, scrollDelta);
                // Update script manager with new values
                scriptManager.setSequencerStepValue(0, ui.getSequencerEditStep(0), 
                                                   dummySteps[ui.getSequencerEditStep(0)] + scrollDelta);
            }
        }
    }
    
    // Handle OK button - advance to next step for editing or go to library
    if (input.isButtonPressed(BTN_OK)) {
        if (scriptManager.isScriptRunning(0)) {
            uint8_t dummy;
            int8_t dummySteps[8];
            if (scriptManager.getSequencerData(0, &dummy, dummySteps)) {
                // Sequencer is running - advance edit step
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
        ui.showScriptLibraryScreen();
    }
    
    // Handle Back button
    if (input.isButtonPressed(BTN_BACK)) {
        currentState = AppState::MAIN_MENU;
        ui.resetMenuTracking();  // Reset for new screen
        ui.showMainMenu();
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
            // Set script type for proper visualization (0=LFO, 1=Sequencer)
            ui.setScriptType(slot, libraryIndex);
            
            Serial.print("Script loaded: ");
            Serial.println(scriptName);
            Serial.print("Script type set to: ");
            Serial.println(libraryIndex);
            
            // Initialize sequencer data if it's a sequencer
            if (libraryIndex == 1) {
                int8_t defaultSteps[8] = {0, 2, 4, 5, 7, 9, 11, 12}; // Ascending scale
                ui.updateScriptSequencer(slot, 0, defaultSteps);
                Serial.println("Sequencer data initialized");
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
        currentState = AppState::SCRIPT_SELECT;
        ui.resetMenuTracking();
        ui.showScriptSelectScreen();
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
