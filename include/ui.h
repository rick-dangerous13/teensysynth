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
    char lastPath[64];  // Previous output for change detection
    bool active;
    uint8_t waveType;  // 0=Sine, 1=Triangle, 2=Square, 3=Saw
    float phase;       // Current phase for animation
};

class UI {
public:
    UI();
    
    void begin(Display* display);
    
    // Screen rendering
    void showWelcomeScreen();
    void showMainMenu();
    void showScriptSelectScreen();
    void showScriptLibraryScreen();  // Browse available scripts
    void showScriptRunningScreen();
    void showSettingsScreen();
    void showAboutScreen();
    
    // Menu navigation
    void scrollMenu(int16_t delta);
    void resetMenuTracking();  // Call when switching screens
    int16_t getSelectedMenuItem();
    int16_t getSelectedSlot();
    void setSelectedSlot(int16_t slot) { selectedScriptSlot = slot; }
    const char* getSelectedScriptPath();
    
    // Script display updates
    void updateScriptStatus(uint8_t slot, bool running);
    void updateScriptInfo(uint8_t slot, const char* name);
    void updateScriptOutput(uint8_t slot, const char* output);  // Update output text
    void updateScriptWaveform(uint8_t slot, uint8_t waveType, float phase);  // Update waveform display
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
    int16_t selectedScriptSlot;  // Which slot we're loading a script into
    
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
