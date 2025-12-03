/**
 * Input Handler Implementation
 * 
 * Handles button debouncing and rotary encoder reading
 */

#include "input.h"

InputHandler::InputHandler() : touchScreen(nullptr) {
    // Initialize all states to default
    for (int i = 0; i < 3; i++) {
        buttonState[i] = false;
        lastButtonState[i] = false;
        buttonPressed[i] = false;
        buttonReleased[i] = false;
        lastDebounceTime[i] = 0;
    }
    
    // Initialize encoder states
    encoderPosition = 0;
    lastEncoderPosition = 0;
    accumulatedDelta = 0;
    lastEncoderCLK = 0;
    lastEncoderDT = 0;
    lastEncoderTime = 0;
    lastStepTime = 0;
    
    // Initialize encoder switch states
    encoderSwitchState = false;
    lastEncoderSwitchState = false;
    encoderSwitchPressed = false;
    encoderSwitchReleased = false;
    encoderSwitchDebounceTime = 0;
    
    // Initialize touch states
    touchActive = false;
    lastTouchActive = false;
    touchPressed = false;
    touchX = 0;
    touchY = 0;
    lastTouchTime = 0;
}

void InputHandler::begin() {
    // Configure button pins with internal pull-up resistors
    pinMode(BTN_OK, INPUT_PULLUP);
    pinMode(BTN_OK2, INPUT_PULLUP);
    pinMode(BTN_BACK, INPUT_PULLUP);
    
    // Configure encoder pins with internal pull-up resistors
    pinMode(ENC_CLK, INPUT_PULLUP);
    pinMode(ENC_DT, INPUT_PULLUP);
    pinMode(ENC_SW, INPUT_PULLUP);
    
    // Initialize touchscreen (polling mode - no IRQ pin)
    Serial.println("Initializing touch screen...");
    
    // Configure CS pin as output and set high (inactive)
    pinMode(TOUCH_CS, OUTPUT);
    digitalWrite(TOUCH_CS, HIGH);
    delay(10);
    
    touchScreen = new XPT2046_Touchscreen(TOUCH_CS);
    if (touchScreen) {
        touchScreen->begin();
        touchScreen->setRotation(3);  // Match display rotation
        Serial.println("Touch screen initialized successfully");
    } else {
        Serial.println("ERROR: Failed to create touch screen object");
    }
    
    Serial.println("Touch initialized");
    
    // Read initial encoder state (2-bit value from both pins)
    uint8_t s = 0;
    if (digitalRead(ENC_CLK)) s |= 1;
    if (digitalRead(ENC_DT)) s |= 2;
    lastEncoderCLK = s;
    lastEncoderDT = 0;  // Use this for previous state
}

void InputHandler::update() {
    // Clear pressed/released flags from previous update
    for (int i = 0; i < 3; i++) {
        buttonPressed[i] = false;
        buttonReleased[i] = false;
    }
    encoderSwitchPressed = false;
    touchPressed = false;
    encoderSwitchReleased = false;
    
    // Update buttons
    updateButton(0, BTN_OK);
    updateButton(1, BTN_OK2);
    updateButton(2, BTN_BACK);
    
    // Update encoder
    updateEncoder();
    updateEncoderSwitch();
    
    // Update touch
    updateTouch();
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
    // Proper quadrature decoding using state machine
    // Based on Paul Stoffregen's Encoder library
    
    // Read current pin states and build 4-bit state value
    uint8_t s = lastEncoderCLK & 3;  // Keep old state in lower 2 bits
    if (digitalRead(ENC_CLK)) s |= 4;  // Add new CLK to bit 2
    if (digitalRead(ENC_DT)) s |= 8;   // Add new DT to bit 3
    
    // State transition lookup - only update on valid transitions
    // This state machine approach filters noise and bouncing
    switch (s) {
        case 0: case 5: case 10: case 15:
            // No movement
            break;
        case 1: case 7: case 8: case 14:
            // Clockwise
            encoderPosition++;
            break;
        case 2: case 4: case 11: case 13:
            // Counter-clockwise
            encoderPosition--;
            break;
        case 3: case 12:
            // Double step clockwise (should not happen with good encoder)
            encoderPosition += 2;
            break;
        case 6: case 9:
            // Double step counter-clockwise
            encoderPosition -= 2;
            break;
    }
    
    // Store new state in lower 2 bits for next comparison
    lastEncoderCLK = (s >> 2);
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
    if (index >= 3) return false;
    
    // Allow encoder button and BTN_OK2 to also act as OK button
    if (button == BTN_OK) {
        uint8_t ok2Index = getButtonIndex(BTN_OK2);
        return buttonPressed[index] || buttonPressed[ok2Index] || encoderSwitchPressed;
    }
    
    return buttonPressed[index];
}

bool InputHandler::isButtonHeld(uint8_t button) {
    uint8_t index = getButtonIndex(button);
    if (index >= 3) return false;
    return buttonState[index];
}

bool InputHandler::isButtonReleased(uint8_t button) {
    uint8_t index = getButtonIndex(button);
    if (index >= 3) return false;
    return buttonReleased[index];
}

int16_t InputHandler::getEncoderDelta() {
    // Calculate encoder delta since last call
    int16_t delta = encoderPosition - lastEncoderPosition;
    
    if (delta != 0) {
        // Accumulate changes
        accumulatedDelta += delta;
        lastEncoderPosition = encoderPosition;
        
        // Only return steps when we have accumulated enough for a full detent
        // DEBO encoder typically needs 4 state transitions per detent
        if (abs(accumulatedDelta) >= 4) {
            int16_t steps = accumulatedDelta / 4;
            accumulatedDelta = accumulatedDelta % 4;  // Keep remainder
            return steps;
        }
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
        case BTN_OK2:  return 1;
        case BTN_BACK: return 2;
        default:       return 255;  // Invalid
    }
}

// ============================================================================
// Touch Input Methods
// ============================================================================

void InputHandler::updateTouch() {
    if (!touchScreen) return;
    
    lastTouchActive = touchActive;
    touchActive = touchScreen->touched();
    
    if (touchActive) {
        TS_Point p = touchScreen->getPoint();
        
        // Map raw touch coordinates to screen coordinates
        touchX = mapTouchX(p.x);
        touchY = mapTouchY(p.y);
        
        // Detect new touch (press event)
        if (!lastTouchActive) {
            touchPressed = true;
            lastTouchTime = millis();
        }
    }
}

int16_t InputHandler::mapTouchX(int16_t rawX) {
    // Map raw touch X to screen X (0-320) based on rotation
    // Rotation 3 (landscape, USB right): map to width
    return map(rawX, TS_MINX, TS_MAXX, 0, SCREEN_WIDTH);
}

int16_t InputHandler::mapTouchY(int16_t rawY) {
    // Map raw touch Y to screen Y (0-240) based on rotation
    // Rotation 3 (landscape, USB right): map to height
    return map(rawY, TS_MINY, TS_MAXY, 0, SCREEN_HEIGHT);
}

bool InputHandler::isTouched() {
    return touchActive;
}

bool InputHandler::wasTouched() {
    return touchPressed;
}

void InputHandler::getTouchPoint(int16_t* x, int16_t* y) {
    if (x) *x = touchX;
    if (y) *y = touchY;
}

TS_Point InputHandler::getRawTouchPoint() {
    if (touchScreen && touchActive) {
        return touchScreen->getPoint();
    }
    TS_Point p;
    p.x = 0;
    p.y = 0;
    p.z = 0;
    return p;
}

void InputHandler::clearTouch() {
    touchPressed = false;
}
