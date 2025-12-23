#ifndef TOUCH_TEST_SCRIPT_H
#define TOUCH_TEST_SCRIPT_H

#include <Arduino.h>

class TouchTestScript {
private:
    bool active;
    unsigned long lastTouchTime;
    
public:
    TouchTestScript();
    
    void start();
    void stop();
    void update();
    bool isActive() const { return active; }
};

#endif // TOUCH_TEST_SCRIPT_H
