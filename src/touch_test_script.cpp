#include "touch_test_script.h"

TouchTestScript::TouchTestScript() : active(false), lastTouchTime(0) {
}

void TouchTestScript::start() {
    active = true;
    lastTouchTime = 0;
    Serial.println("===== TOUCH CALIBRATION TEST =====");
    Serial.println("Touch the corners and center of the screen:");
    Serial.println("1. Top-left corner");
    Serial.println("2. Top-right corner");
    Serial.println("3. Bottom-left corner");
    Serial.println("4. Bottom-right corner");
    Serial.println("5. Center of screen");
}

void TouchTestScript::stop() {
    active = false;
    Serial.println("===== TOUCH TEST STOPPED =====");
}

void TouchTestScript::update() {
    // Nothing to update - this is just a passive test mode
}
