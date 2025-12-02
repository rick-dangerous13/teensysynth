/**
 * Input Handler
 * 
 * Handles 2 buttons (OK, Back) and rotary encoder for scrolling
 */

#ifndef INPUT_H
#define INPUT_H

#include <Arduino.h>
#include <XPT2046_Touchscreen.h>
#include "config.h"

class InputHandler {
public:
    InputHandler();
    
    void begin();
    void update();
    
    // Button state queries (OK and Back buttons)
    bool isButtonPressed(uint8_t button);
    bool isButtonHeld(uint8_t button);
    bool isButtonReleased(uint8_t button);
    
    // Encoder queries
    int16_t getEncoderDelta();     // Returns rotation delta since last call (-n, 0, +n)
    bool isEncoderPressed();       // Encoder switch pressed
    bool isEncoderReleased();      // Encoder switch released
    
    // Touch queries
    bool isTouched();              // Is screen currently touched?
    bool wasTouched();             // Was screen touched since last check?
    void getTouchPoint(int16_t* x, int16_t* y);  // Get current touch coordinates
    TS_Point getRawTouchPoint();   // Get raw touch coordinates for calibration
    void clearTouch();             // Clear touch state
    
private:
    // Button states (OK and Back buttons)
    bool buttonState[2];
    bool lastButtonState[2];
    bool buttonPressed[2];
    bool buttonReleased[2];
    unsigned long lastDebounceTime[2];
    
    // Encoder switch state
    bool encoderSwitchState;
    bool lastEncoderSwitchState;
    bool encoderSwitchPressed;
    bool encoderSwitchReleased;
    unsigned long encoderSwitchDebounceTime;
    
    // Encoder rotation state
    volatile int16_t encoderPosition;
    int16_t lastEncoderPosition;
    int16_t accumulatedDelta;      // Accumulate small movements
    uint8_t lastEncoderCLK;
    uint8_t lastEncoderDT;
    unsigned long lastEncoderTime;
    unsigned long lastStepTime;    // Track time of last menu step
    
    // Touch state
    XPT2046_Touchscreen* touchScreen;
    bool touchActive;
    bool lastTouchActive;
    bool touchPressed;
    int16_t touchX;
    int16_t touchY;
    unsigned long lastTouchTime;
    
    // Helper methods
    uint8_t getButtonIndex(uint8_t button);
    void updateButton(uint8_t index, uint8_t pin);
    void updateEncoder();
    void updateEncoderSwitch();
    void updateTouch();
    int16_t mapTouchX(int16_t rawX);
    int16_t mapTouchY(int16_t rawY);
};

#endif // INPUT_H
