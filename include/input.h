/**
 * Input Handler
 * 
 * Handles 2 buttons (OK, Back) and potentiometer for scrolling
 */

#ifndef INPUT_H
#define INPUT_H

#include <Arduino.h>
#include "config.h"

class InputHandler {
public:
    InputHandler();
    
    void begin();
    void update();
    
    // Button state queries
    bool isButtonPressed(uint8_t button);
    bool isButtonHeld(uint8_t button);
    bool isButtonReleased(uint8_t button);
    
    // Potentiometer queries
    int16_t getPotValue();
    int16_t getScrollDelta();  // Returns -1, 0, or 1 based on pot movement
    
private:
    // Button states
    bool buttonState[2];
    bool lastButtonState[2];
    bool buttonPressed[2];
    bool buttonReleased[2];
    unsigned long lastDebounceTime[2];
    
    // Potentiometer state
    int16_t potValue;
    int16_t lastPotValue;
    int16_t scrollPosition;
    int16_t lastScrollPosition;
    
    // Helper methods
    uint8_t getButtonIndex(uint8_t button);
    void updateButton(uint8_t index, uint8_t pin);
    void updatePotentiometer();
};

#endif // INPUT_H
