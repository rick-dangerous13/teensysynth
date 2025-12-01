/**
 * Input Handler Implementation
 * 
 * Handles button debouncing and rotary encoder reading
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
    
    // Initialize encoder states
    encoderPosition = 0;
    lastEncoderPosition = 0;
    lastEncoderCLK = HIGH;
    lastEncoderDT = HIGH;
    lastEncoderTime = 0;
    
    // Initialize encoder switch states
    encoderSwitchState = false;
    lastEncoderSwitchState = false;
    encoderSwitchPressed = false;
    encoderSwitchReleased = false;
    encoderSwitchDebounceTime = 0;
}

void InputHandler::begin() {
    // Configure button pins with internal pull-up resistors
    pinMode(BTN_OK, INPUT_PULLUP);
    pinMode(BTN_BACK, INPUT_PULLUP);
    
    // Configure encoder pins with internal pull-up resistors
    pinMode(ENC_CLK, INPUT_PULLUP);
    pinMode(ENC_DT, INPUT_PULLUP);
    pinMode(ENC_SW, INPUT_PULLUP);
    
    // Read initial encoder pin states
    lastEncoderCLK = digitalRead(ENC_CLK);
    lastEncoderDT = digitalRead(ENC_DT);
}

void InputHandler::update() {
    // Clear pressed/released flags from previous update
    for (int i = 0; i < 2; i++) {
        buttonPressed[i] = false;
        buttonReleased[i] = false;
    }
    encoderSwitchPressed = false;
    encoderSwitchReleased = false;
    
    // Update buttons
    updateButton(0, BTN_OK);
    updateButton(1, BTN_BACK);
    
    // Update encoder
    updateEncoder();
    updateEncoderSwitch();
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

void InputHandler::updateEncoder() {
    // Read current encoder pin states
    uint8_t currentCLK = digitalRead(ENC_CLK);
    uint8_t currentDT = digitalRead(ENC_DT);
    
    // Detect state changes with debouncing
    unsigned long currentTime = millis();
    if (currentTime - lastEncoderTime < ENC_DEBOUNCE_MS) {
        return;  // Too soon, ignore
    }
    
    // Check if CLK pin changed (falling edge)
    if (currentCLK != lastEncoderCLK && currentCLK == LOW) {
        // CLK went from HIGH to LOW
        // Check DT to determine direction
        if (currentDT == HIGH) {
            // Clockwise rotation
            encoderPosition++;
        } else {
            // Counter-clockwise rotation
            encoderPosition--;
        }
        lastEncoderTime = currentTime;
    }
    
    // Save current states
    lastEncoderCLK = currentCLK;
    lastEncoderDT = currentDT;
}

void InputHandler::updateEncoderSwitch() {
    // Read encoder switch state (active low with pull-up)
    bool reading = !digitalRead(ENC_SW);
    
    // Check if switch state has changed
    if (reading != lastEncoderSwitchState) {
        encoderSwitchDebounceTime = millis();
    }
    
    // Apply debouncing
    if ((millis() - encoderSwitchDebounceTime) > DEBOUNCE_DELAY) {
        // State has been stable long enough
        if (reading != encoderSwitchState) {
            encoderSwitchState = reading;
            
            if (encoderSwitchState) {
                encoderSwitchPressed = true;
            } else {
                encoderSwitchReleased = true;
            }
        }
    }
    
    lastEncoderSwitchState = reading;
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

int16_t InputHandler::getEncoderDelta() {
    // Calculate encoder delta since last call
    int16_t delta = encoderPosition - lastEncoderPosition;
    lastEncoderPosition = encoderPosition;
    
    // Normalize to menu steps (divide by pulses per notch/detent)
    // This gives us smoother control - one menu step per physical click
    if (abs(delta) >= ENC_STEPS_PER_NOTCH) {
        int16_t steps = delta / ENC_STEPS_PER_NOTCH;
        return steps;
    }
    
    return 0;
}

bool InputHandler::isEncoderPressed() {
    return encoderSwitchPressed;
}

bool InputHandler::isEncoderReleased() {
    return encoderSwitchReleased;
}

uint8_t InputHandler::getButtonIndex(uint8_t button) {
    switch (button) {
        case BTN_OK:   return 0;
        case BTN_BACK: return 1;
        default:       return 255;  // Invalid
    }
}
