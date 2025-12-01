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
        ui.showMainMenu();  // Redraw the menu
    }
    
    // Handle button presses
    if (input.isButtonPressed(BTN_OK)) {
        int selection = ui.getSelectedMenuItem();
        switch (selection) {
            case 0: // Scripts
                currentState = AppState::SCRIPT_SELECT;
                ui.showScriptSelectScreen();
                break;
            case 1: // Settings
                currentState = AppState::SETTINGS;
                ui.showSettingsScreen();
                break;
            case 2: // About
                ui.showAboutScreen();
                break;
        }
    }
}

void handleScriptSelectState() {
    // Handle scrolling
    int scrollDelta = input.getEncoderDelta();
    if (scrollDelta != 0) {
        ui.scrollMenu(scrollDelta);
        ui.showScriptSelectScreen();  // Redraw
    }
    
    // Handle OK button - load script
    if (input.isButtonPressed(BTN_OK)) {
        int slot = ui.getSelectedSlot();
        if (slot >= 0 && slot < MAX_SCRIPTS) {
            scriptManager.loadScript(slot, ui.getSelectedScriptPath());
            ui.updateScriptStatus(slot, true);
        }
    }
    
    // Handle Back button
    if (input.isButtonPressed(BTN_BACK)) {
        currentState = AppState::MAIN_MENU;
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
    
    // Handle Back button
    if (input.isButtonPressed(BTN_BACK)) {
        currentState = AppState::MAIN_MENU;
        ui.showMainMenu();
    }
}

void handleSettingsState() {
    // Handle scrolling
    int scrollDelta = input.getEncoderDelta();
    if (scrollDelta != 0) {
        ui.scrollMenu(scrollDelta);
        ui.showSettingsScreen();  // Redraw
    }
    
    // Handle OK button
    if (input.isButtonPressed(BTN_OK)) {
        ui.toggleSettingValue();
    }
    
    // Handle Back button
    if (input.isButtonPressed(BTN_BACK)) {
        currentState = AppState::MAIN_MENU;
        ui.showMainMenu();
    }
}
