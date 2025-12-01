/**
 * UI System Implementation
 * 
 * Norns-style graphical user interface
 */

#include "ui.h"
#include <string.h>
#include <stdio.h>
#include <math.h>

UI::UI() : display(nullptr), menuSelection(0), lastMenuSelection(-1), menuItemCount(0), scrollOffset(0), selectedScriptSlot(0) {
    // Initialize script slots
    for (int i = 0; i < MAX_SCRIPTS; i++) {
        strcpy(scriptSlots[i].name, "empty");
        scriptSlots[i].path[0] = '\0';
        scriptSlots[i].lastPath[0] = '\0';
        scriptSlots[i].active = false;
        scriptSlots[i].waveType = 0;
        scriptSlots[i].phase = 0.0f;
    }
    
    // Initialize menu items
    for (int i = 0; i < MAX_MENU_ITEMS; i++) {
        menuItems[i].label = nullptr;
        menuItems[i].enabled = false;
    }
}

void UI::begin(Display* disp) {
    display = disp;
}

void UI::showWelcomeScreen() {
    if (!display) return;
    
    display->clear();
    
    // Draw Norns-style minimalist welcome screen
    // Large centered title
    display->drawTextCentered(70, "TEENSYSYNTH", COLOR_FG, FONT_LARGE);
    
    // Decorative line
    display->drawLine(80, 110, 240, 110, COLOR_ACCENT);
    
    // Subtitle
    display->drawTextCentered(130, "eurorack synthesizer", COLOR_DIM, FONT_MEDIUM);
    
    // Version info at bottom
    display->drawTextCentered(200, "v1.0.0", COLOR_DIM, FONT_SMALL);
    
    // "Press OK to continue" hint
    display->drawTextCentered(220, "press OK to continue", COLOR_DIM, FONT_SMALL);
}

void UI::showMainMenu() {
    if (!display) return;
    
    // Set up menu items
    menuItems[0].label = "SCRIPTS";
    menuItems[0].enabled = true;
    menuItems[1].label = "SETTINGS";
    menuItems[1].enabled = true;
    menuItems[2].label = "ABOUT";
    menuItems[2].enabled = true;
    menuItemCount = 3;
    
    // Only do full redraw if this is initial display
    if (lastMenuSelection == -1) {
        display->clear();
        drawHeader("TEENSYSYNTH");
        drawFooter("OK: select", "");
        
        // Draw all menu items
        int16_t startY = 50;
        for (int i = 0; i < menuItemCount; i++) {
            drawMenuItem(startY + i * MENU_ITEM_H, menuItems[i].label, i == menuSelection);
        }
        display->drawScrollIndicator(50, menuItemCount, 6, menuSelection);
    } else if (lastMenuSelection != menuSelection) {
        // Only redraw the changed menu items
        int16_t startY = 50;
        drawMenuItem(startY + lastMenuSelection * MENU_ITEM_H, menuItems[lastMenuSelection].label, false);
        drawMenuItem(startY + menuSelection * MENU_ITEM_H, menuItems[menuSelection].label, true);
        display->drawScrollIndicator(50, menuItemCount, 6, menuSelection);
    }
    
    lastMenuSelection = menuSelection;
}

void UI::showScriptSelectScreen() {
    if (!display) return;
    
    menuItemCount = MAX_SCRIPTS;  // 4 script slots
    
    // Only do full redraw if this is initial display
    if (lastMenuSelection == -1) {
        display->clear();
        display->drawQuadrantDividers();
        drawFooter("OK: load", "BACK: menu");
        lastMenuSelection = menuSelection;
    }
    
    // Always redraw all script slots to show live updates
    for (uint8_t i = 0; i < MAX_SCRIPTS; i++) {
        drawScriptSlot(i, i == menuSelection);
    }
}

void UI::showScriptLibraryScreen() {
    if (!display) return;
    
    // This will be populated from ScriptManager
    menuItemCount = 4;  // Will be set from scriptManager
    
    // Only do full redraw if this is initial display
    if (lastMenuSelection == -1) {
        display->clear();
        
        char headerText[32];
        snprintf(headerText, sizeof(headerText), "LOAD > SLOT %d", selectedScriptSlot + 1);
        drawHeader(headerText);
        
        drawFooter("OK: load", "BACK: cancel");
        
        // Draw library items (will be updated from main loop)
        int16_t startY = 50;
        const char* tempItems[] = {"LFO", "Sequencer", "Envelope", "Clock"};
        for (int i = 0; i < 4; i++) {
            drawMenuItem(startY + i * MENU_ITEM_H, tempItems[i], i == menuSelection);
        }
    } else if (lastMenuSelection != menuSelection) {
        // Only redraw the changed menu items
        int16_t startY = 50;
        const char* tempItems[] = {"LFO", "Sequencer", "Envelope", "Clock"};
        drawMenuItem(startY + lastMenuSelection * MENU_ITEM_H, tempItems[lastMenuSelection], false);
        drawMenuItem(startY + menuSelection * MENU_ITEM_H, tempItems[menuSelection], true);
    }
    
    lastMenuSelection = menuSelection;
}

void UI::showScriptRunningScreen() {
    if (!display) return;
    
    display->clear();
    
    // Draw 4-quadrant view for running scripts
    display->drawQuadrantDividers();
    
    for (uint8_t i = 0; i < MAX_SCRIPTS; i++) {
        display->setClipRegion(i);
        
        // Draw script header
        int16_t x = (i % 2) * (SCREEN_WIDTH / 2) + 5;
        int16_t y = (i / 2) * (SCREEN_HEIGHT / 2) + 5;
        
        if (scriptSlots[i].active) {
            display->drawText(x, y, scriptSlots[i].name, COLOR_ACCENT, FONT_SMALL);
        } else {
            display->drawText(x, y, "empty", COLOR_DIM, FONT_SMALL);
        }
    }
    
    display->clearClipRegion();
}

void UI::showSettingsScreen() {
    if (!display) return;
    
    // Setting items
    const char* settings[] = {
        "Audio Output",
        "MIDI Channel",
        "Display Brightness",
        "Script Auto-load"
    };
    
    menuItemCount = 4;
    
    // Only do full redraw if this is initial display
    if (lastMenuSelection == -1) {
        display->clear();
        drawHeader("SETTINGS");
        drawFooter("OK: edit", "BACK: menu");
        
        // Draw all menu items
        int16_t startY = 50;
        for (int i = 0; i < menuItemCount; i++) {
            drawMenuItem(startY + i * MENU_ITEM_H, settings[i], i == menuSelection);
        }
    } else if (lastMenuSelection != menuSelection) {
        // Only redraw the changed menu items
        int16_t startY = 50;
        drawMenuItem(startY + lastMenuSelection * MENU_ITEM_H, settings[lastMenuSelection], false);
        drawMenuItem(startY + menuSelection * MENU_ITEM_H, settings[menuSelection], true);
    }
    
    lastMenuSelection = menuSelection;
}

void UI::showAboutScreen() {
    if (!display) return;
    
    display->clear();
    
    // Draw header
    drawHeader("ABOUT");
    
    // About info
    display->drawText(MARGIN, 50, "TeensySynth", COLOR_FG, FONT_MEDIUM);
    display->drawText(MARGIN, 75, "Eurorack Synthesizer", COLOR_DIM, FONT_SMALL);
    display->drawText(MARGIN, 95, "Norns-compatible scripts", COLOR_DIM, FONT_SMALL);
    display->drawText(MARGIN, 115, "4-voice multitasking", COLOR_DIM, FONT_SMALL);
    
    // Hardware info
    display->drawText(MARGIN, 145, "Hardware:", COLOR_ACCENT, FONT_SMALL);
    display->drawText(MARGIN, 165, "Teensy 4.1 @ 600MHz", COLOR_DIM, FONT_SMALL);
    display->drawText(MARGIN, 185, "ILI9341 320x240 TFT", COLOR_DIM, FONT_SMALL);
    
    // Draw footer
    drawFooter("", "BACK: menu");
}

void UI::scrollMenu(int16_t delta) {
    menuSelection += delta;
    
    // Wrap around
    if (menuSelection < 0) {
        menuSelection = menuItemCount - 1;
    }
    if (menuSelection >= menuItemCount) {
        menuSelection = 0;
    }
}

void UI::resetMenuTracking() {
    lastMenuSelection = -1;  // Force full redraw on next show
    menuSelection = 0;       // Reset to first item
}

int16_t UI::getSelectedMenuItem() {
    return menuSelection;
}

int16_t UI::getSelectedSlot() {
    return menuSelection;
}

const char* UI::getSelectedScriptPath() {
    if (menuSelection >= 0 && menuSelection < MAX_SCRIPTS) {
        return scriptSlots[menuSelection].path;
    }
    return "";
}

void UI::updateScriptStatus(uint8_t slot, bool running) {
    if (slot < MAX_SCRIPTS) {
        scriptSlots[slot].active = running;
    }
}

void UI::updateScriptInfo(uint8_t slot, const char* name) {
    if (slot < MAX_SCRIPTS && name != nullptr) {
        strncpy(scriptSlots[slot].name, name, sizeof(scriptSlots[slot].name) - 1);
        scriptSlots[slot].name[sizeof(scriptSlots[slot].name) - 1] = '\0';
    }
}

void UI::updateScriptOutput(uint8_t slot, const char* output) {
    if (slot < MAX_SCRIPTS && output != nullptr) {
        // Store output in path field for display
        strncpy(scriptSlots[slot].path, output, sizeof(scriptSlots[slot].path) - 1);
        scriptSlots[slot].path[sizeof(scriptSlots[slot].path) - 1] = '\0';
    }
}

void UI::updateScriptWaveform(uint8_t slot, uint8_t waveType, float phase) {
    if (slot < MAX_SCRIPTS) {
        scriptSlots[slot].waveType = waveType;
        scriptSlots[slot].phase = phase;
    }
}

void UI::updateScriptDisplay(uint8_t slot, const char* output) {
    if (slot >= MAX_SCRIPTS || !display) return;
    
    // Set clipping to script's quadrant
    display->setClipRegion(slot);
    
    // Calculate position
    int16_t x = (slot % 2) * (SCREEN_WIDTH / 2) + 5;
    int16_t y = (slot / 2) * (SCREEN_HEIGHT / 2) + 20;
    
    // Clear previous output area
    display->fillRect(x, y, SCREEN_WIDTH / 2 - 10, SCREEN_HEIGHT / 2 - 25, COLOR_BG);
    
    // Draw new output
    display->drawText(x, y, output, COLOR_FG, FONT_SMALL);
    
    display->clearClipRegion();
}

void UI::toggleSettingValue() {
    // Placeholder for settings toggle functionality
    // Would be expanded to handle specific settings
}

void UI::drawMenuItem(int16_t y, const char* label, bool selected) {
    if (!display) return;
    
    if (selected) {
        // Draw selection highlight
        display->fillRect(MARGIN, y - 2, SCREEN_WIDTH - 2 * MARGIN - 10, MENU_ITEM_H, COLOR_FG);
        
        // Selection indicator with label
        display->drawText(MARGIN + 5, y, ">", COLOR_BG, FONT_MEDIUM);
        display->drawText(MARGIN + 15, y, label, COLOR_BG, FONT_MEDIUM);
    } else {
        // Clear the highlight area first
        display->fillRect(MARGIN, y - 2, SCREEN_WIDTH - 2 * MARGIN - 10, MENU_ITEM_H, COLOR_BG);
        
        // Draw unselected label
        display->drawText(MARGIN + 15, y, label, COLOR_DIM, FONT_MEDIUM);
    }
}

void UI::drawScriptSlot(uint8_t slot, bool selected) {
    if (!display || slot >= MAX_SCRIPTS) return;
    
    // Calculate quadrant position
    int16_t x = (slot % 2) * (SCREEN_WIDTH / 2);
    int16_t y = (slot / 2) * (SCREEN_HEIGHT / 2);
    int16_t w = SCREEN_WIDTH / 2 - 2;
    int16_t h = SCREEN_HEIGHT / 2 - 2;
    
    // Draw slot border (selection indicator)
    if (selected) {
        display->drawRect(x + 2, y + 2, w - 2, h - 2, COLOR_ACCENT);
    } else {
        display->drawRect(x + 2, y + 2, w - 2, h - 2, COLOR_BG);
    }
    
    // Draw slot number
    char slotNum[4];
    snprintf(slotNum, sizeof(slotNum), "%d", slot + 1);
    display->drawText(x + 5, y + 5, slotNum, COLOR_DIM, FONT_SMALL);
    
    // Draw script name or "empty"
    if (scriptSlots[slot].active) {
        display->drawText(x + 20, y + 5, scriptSlots[slot].name, COLOR_FG, FONT_MEDIUM);
        
        // Draw running indicator (cyan dot)
        display->fillCircle(x + w - 10, y + 10, 4, COLOR_ACCENT);
        
        // Check if parameters changed
        bool parametersChanged = strcmp(scriptSlots[slot].path, scriptSlots[slot].lastPath) != 0;
        
        // Draw waveform visualization (double the height for better visibility)
        int16_t waveX = x + 8;
        int16_t waveY = y + 25;
        int16_t waveW = w - 20;
        int16_t waveH = 60;  // Doubled from 30
        int16_t waveCenterY = waveY + waveH / 2;
        
        // Only clear and redraw if parameters changed
        if (parametersChanged) {
            // Clear only the content area (waveform + text), not the borders
            display->fillRect(x + 6, waveY - 2, w - 12, h - 22, COLOR_BG);
            
            // Update last path
            strncpy(scriptSlots[slot].lastPath, scriptSlots[slot].path, sizeof(scriptSlots[slot].lastPath) - 1);
            scriptSlots[slot].lastPath[sizeof(scriptSlots[slot].lastPath) - 1] = '\0';
        } else {
            // Just clear the waveform area for smooth animation
            display->fillRect(waveX, waveY, waveW, waveH, COLOR_BG);
        }
        
        // Draw waveform axis
        display->drawLine(waveX, waveCenterY, waveX + waveW, waveCenterY, COLOR_DIM);
        
        // Draw waveform based on type
        uint8_t waveType = scriptSlots[slot].waveType;
        float phase = scriptSlots[slot].phase;
        
        for (int16_t i = 0; i < waveW; i++) {
            float t = (float)i / waveW * 2.0f * PI + phase;
            float value = 0.0f;
            
            switch (waveType) {
                case 0: // Sine
                    value = sin(t);
                    break;
                case 1: // Triangle
                    {
                        float normalized = fmod(t, 2.0f * PI) / (2.0f * PI);
                        value = (normalized < 0.5f) ? (normalized * 4.0f - 1.0f) : (3.0f - normalized * 4.0f);
                    }
                    break;
                case 2: // Square
                    value = (fmod(t, 2.0f * PI) < PI) ? 1.0f : -1.0f;
                    break;
                case 3: // Sawtooth
                    value = 2.0f * (fmod(t, 2.0f * PI) / (2.0f * PI)) - 1.0f;
                    break;
            }
            
            int16_t pixelY = waveCenterY - (int16_t)(value * waveH * 0.4f);
            display->drawPixel(waveX + i, pixelY, COLOR_ACCENT);
        }
        
        // Draw parameters below waveform
        int16_t textY = waveY + waveH + 5;
        int16_t lineHeight = 10;
        
        // Parse and display parameters (only if they changed)
        if (parametersChanged && strlen(scriptSlots[slot].path) > 0) {
            char outputCopy[128];
            strncpy(outputCopy, scriptSlots[slot].path, sizeof(outputCopy) - 1);
            outputCopy[sizeof(outputCopy) - 1] = '\0';
            
            char* line = strtok(outputCopy, "\n");
            int lineCount = 0;
            
            // Skip first line if it matches the script name (avoid duplication)
            if (line != nullptr && strcmp(line, scriptSlots[slot].name) == 0) {
                line = strtok(nullptr, "\n");
            }
            
            // Display up to 3 parameter lines
            while (line != nullptr && textY < (y + h - 5) && lineCount < 3) {
                display->drawText(x + 8, textY, line, COLOR_FG, FONT_SMALL);
                textY += lineHeight;
                line = strtok(nullptr, "\n");
                lineCount++;
            }
        }
    } else {
        display->drawText(x + 20, y + 5, "empty", COLOR_DIM, FONT_MEDIUM);
    }
}

void UI::drawHeader(const char* title) {
    if (!display) return;
    
    // Draw title
    display->drawText(MARGIN, MARGIN, title, COLOR_FG, FONT_MEDIUM);
    
    // Draw separator line
    display->drawLine(MARGIN, 35, SCREEN_WIDTH - MARGIN, 35, COLOR_DIM);
}

void UI::drawFooter(const char* leftLabel, const char* rightLabel) {
    if (!display) return;
    
    // Draw separator line
    display->drawLine(MARGIN, SCREEN_HEIGHT - 25, SCREEN_WIDTH - MARGIN, SCREEN_HEIGHT - 25, COLOR_DIM);
    
    // Draw left label (OK button hint)
    display->drawText(MARGIN, SCREEN_HEIGHT - 18, leftLabel, COLOR_DIM, FONT_SMALL);
    
    // Draw right label (Back button hint) - right-aligned
    if (rightLabel && strlen(rightLabel) > 0) {
        int16_t x1, y1;
        uint16_t w, h;
        display->getTextBounds(rightLabel, 0, 0, &x1, &y1, &w, &h, FONT_SMALL);
        display->drawText(SCREEN_WIDTH - MARGIN - w, SCREEN_HEIGHT - 18, rightLabel, COLOR_DIM, FONT_SMALL);
    }
}
