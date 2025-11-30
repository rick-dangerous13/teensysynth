/**
 * Input Handler Implementation
 * 
 * Handles button debouncing and potentiometer smoothing
 */

#include "input.h"

InputHandler::InputHandler() {
    // Initialize all states to default
    for (int i = 0; i < 2; i++) {
        buttonState[i] = false;
        lastButtonState[i] = false;
        buttonPressed[i] = false;
        buttonReleased[i] = false;
        lastDebounceTime[i] = 0;
    }
    potValue = 0;
    lastPotValue = 0;
    scrollPosition = 0;
    lastScrollPosition = 0;
}

void InputHandler::begin() {
    // Configure button pins with internal pull-up resistors
    pinMode(BTN_OK, INPUT_PULLUP);
    pinMode(BTN_BACK, INPUT_PULLUP);
    
    // Configure analog input for potentiometer
    pinMode(POT_SCROLL, INPUT);
    analogReadResolution(10);  // 10-bit ADC (0-1023)
    
    // Read initial potentiometer value
    potValue = analogRead(POT_SCROLL);
    lastPotValue = potValue;
    
    // Initialize scroll position based on pot value
    scrollPosition = map(potValue, POT_MIN, POT_MAX, 0, 100);
    lastScrollPosition = scrollPosition;
}

void InputHandler::update() {
    // Clear pressed/released flags from previous update
    for (int i = 0; i < 2; i++) {
        buttonPressed[i] = false;
        buttonReleased[i] = false;
    }
    
    // Update buttons
    updateButton(0, BTN_OK);
    updateButton(1, BTN_BACK);
    
    // Update potentiometer
    updatePotentiometer();
}

void InputHandler::updateButton(uint8_t index, uint8_t pin) {
    // Read button state (active low with pull-up)
    bool reading = !digitalRead(pin);
    
    // Check if button state has changed
    if (reading != lastButtonState[index]) {
        lastDebounceTime[index] = millis();
    }
    
    // Apply debouncing
    if ((millis() - lastDebounceTime[index]) > DEBOUNCE_DELAY) {
        // State has been stable long enough
        if (reading != buttonState[index]) {
            buttonState[index] = reading;
            
            if (buttonState[index]) {
                buttonPressed[index] = true;
            } else {
                buttonReleased[index] = true;
            }
        }
    }
    
    lastButtonState[index] = reading;
}

void InputHandler::updatePotentiometer() {
    // Read and smooth potentiometer value
    int16_t rawValue = analogRead(POT_SCROLL);
    
    // Apply simple exponential smoothing
    potValue = (potValue * 3 + rawValue) / 4;
    
    // Convert to scroll position (0-100)
    scrollPosition = map(potValue, POT_MIN, POT_MAX, 0, 100);
}

bool InputHandler::isButtonPressed(uint8_t button) {
    uint8_t index = getButtonIndex(button);
    if (index >= 2) return false;
    return buttonPressed[index];
}

bool InputHandler::isButtonHeld(uint8_t button) {
    uint8_t index = getButtonIndex(button);
    if (index >= 2) return false;
    return buttonState[index];
}

bool InputHandler::isButtonReleased(uint8_t button) {
    uint8_t index = getButtonIndex(button);
    if (index >= 2) return false;
    return buttonReleased[index];
}

int16_t InputHandler::getPotValue() {
    return potValue;
}

int16_t InputHandler::getScrollDelta() {
    // Calculate scroll delta based on position change
    int16_t delta = 0;
    
    // Check if position has changed significantly
    int16_t diff = scrollPosition - lastScrollPosition;
    
    if (abs(diff) > 2) {  // Threshold to prevent jitter
        delta = (diff > 0) ? 1 : -1;
        lastScrollPosition = scrollPosition;
    }
    
    return delta;
}

uint8_t InputHandler::getButtonIndex(uint8_t button) {
    switch (button) {
        case BTN_OK:   return 0;
        case BTN_BACK: return 1;
        default:       return 255;  // Invalid
    }
}
