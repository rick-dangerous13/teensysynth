/**
 * UI System
 * 
 * Norns-style graphical user interface for TeensySynth
 */

#ifndef UI_H
#define UI_H

#include <Arduino.h>
#include "config.h"
#include "display.h"

// Menu item structure
struct MenuItem {
    const char* label;
    bool enabled;
};

// Script slot structure  
struct ScriptSlot {
    char name[32];
    char path[64];
    bool active;
};

class UI {
public:
    UI();
    
    void begin(Display* display);
    
    // Screen rendering
    void showWelcomeScreen();
    void showMainMenu();
    void showScriptSelectScreen();
    void showScriptRunningScreen();
    void showSettingsScreen();
    void showAboutScreen();
    
    // Menu navigation
    void scrollMenu(int16_t delta);
    void resetMenuTracking();  // Call when switching screens
    int16_t getSelectedMenuItem();
    int16_t getSelectedSlot();
    const char* getSelectedScriptPath();
    
    // Script display updates
    void updateScriptStatus(uint8_t slot, bool running);
    void updateScriptDisplay(uint8_t slot, const char* output);
    
    // Settings control
    void toggleSettingValue();
    
private:
    Display* display;
    
    // Menu state
    int16_t menuSelection;
    int16_t lastMenuSelection;  // Track previous selection for partial updates
    int16_t menuItemCount;
    int16_t scrollOffset;
    
    // Available menu items
    static const int MAX_MENU_ITEMS = 10;
    MenuItem menuItems[MAX_MENU_ITEMS];
    
    // Script slots for multitasking
    ScriptSlot scriptSlots[MAX_SCRIPTS];
    
    // Helper drawing methods
    void drawMenuItem(int16_t y, const char* label, bool selected);
    void drawScriptSlot(uint8_t slot, bool selected);
    void drawHeader(const char* title);
    void drawFooter(const char* leftLabel, const char* rightLabel);
};

#endif // UI_H
