/**
 * UI System Implementation
 * 
 * Norns-style graphical user interface
 */

#include "ui.h"
#include <string.h>
#include <stdio.h>
#include <math.h>

UI::UI() : display(nullptr), clockTempo(DEFAULT_CLOCK_BPM), multitaskingMode(false), menuSelection(0), lastMenuSelection(-1), menuItemCount(0), scrollOffset(0), selectedScriptSlot(0) {
    // Initialize script slots
    for (int i = 0; i < MAX_SCRIPTS; i++) {
        strcpy(scriptSlots[i].name, "empty");
        scriptSlots[i].path[0] = '\0';
        scriptSlots[i].lastPath[0] = '\0';
        scriptSlots[i].active = false;
        scriptSlots[i].scriptType = 255;  // No script
        scriptSlots[i].waveType = 0;
        scriptSlots[i].phase = 0.0f;
        scriptSlots[i].lfoFrequency = 1.0f;
        scriptSlots[i].lfoLevel = 5.0f;
        scriptSlots[i].lfoEditParam = 0;
        scriptSlots[i].lastLfoWaveType = 255;
        scriptSlots[i].lastLfoFrequency = -1.0f;
        scriptSlots[i].lastLfoLevel = -1.0f;
        scriptSlots[i].lastLfoEditParam = 255;
        scriptSlots[i].seqCurrentStep = 0;
        scriptSlots[i].seqEditStep = 0;
        scriptSlots[i].seqEditingDuration = false;
        scriptSlots[i].seqEditMode = 0;
        scriptSlots[i].seqDirection = 0;
        scriptSlots[i].seqCurrentBeat = 0;
        scriptSlots[i].seqSteamTrigger = false;
        scriptSlots[i].lastSeqCurrentStep = 255;  // 255 = uninitialized
        scriptSlots[i].lastSeqEditStep = 255;
        scriptSlots[i].lastSeqEditingDuration = false;
        scriptSlots[i].lastSeqEditMode = 0;
        scriptSlots[i].lastSeqDirection = 255;
        scriptSlots[i].lastSeqCurrentBeat = 255;
        for (int j = 0; j < 8; j++) {
            scriptSlots[i].seqStepValues[j] = 0;
            scriptSlots[i].seqStepDurations[j] = 1;
            scriptSlots[i].seqGateModes[j] = 0;
            scriptSlots[i].lastSeqStepValues[j] = 0;
            scriptSlots[i].lastSeqStepDurations[j] = 1;
            scriptSlots[i].lastSeqGateModes[j] = 0;
        }
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
    display->drawTextCentered(70, "POLYPHONION", COLOR_FG, FONT_LARGE);
    
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
        drawHeader("POLYPHONION");
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
    
    menuItemCount = 1;  // Only slot 1 (for now, until we have 4 pairs of CV/Gate jacks)
    
    // Only do full redraw if this is initial display
    if (lastMenuSelection == -1) {
        display->clear();
        if (multitaskingMode) {
            display->drawQuadrantDividers();
        }
        drawFooter("OK: load", "BACK: menu");
        lastMenuSelection = menuSelection;
    }
    
    // Always redraw script slots - only slot 0 for now
    menuSelection = 0;  // Force selection to slot 0
    selectedScriptSlot = 0;
    drawScriptSlot(0, true);  // Always selected since it's the only available slot
}

void UI::showScriptLibraryScreen() {
    if (!display) return;
    
    // Note: menuItemCount should be set from main.cpp using script manager
    // Default to 5 if not set
    if (menuItemCount == 0) menuItemCount = 5;
    
    // Only do full redraw if this is initial display
    if (lastMenuSelection == -1) {
        display->clear();
        
        char headerText[32];
        snprintf(headerText, sizeof(headerText), "LOAD > SLOT %d", selectedScriptSlot + 1);
        drawHeader(headerText);
        
        drawFooter("OK: load", "BACK: cancel");
        
        // Draw library items (temp names, should be updated from main loop)
        int16_t startY = 50;
        const char* tempItems[] = {"LFO", "8 Step Sequencer", "SteampunQuencer", "Envelope", "Clock"};
        for (int i = 0; i < menuItemCount && i < 5; i++) {
            drawMenuItem(startY + i * MENU_ITEM_H, tempItems[i], i == menuSelection);
        }
    } else if (lastMenuSelection != menuSelection) {
        // Only redraw the changed menu items
        int16_t startY = 50;
        const char* tempItems[] = {"LFO", "8 Step Sequencer", "SteampunQuencer", "Envelope", "Clock"};
        if (lastMenuSelection < 5) {
            drawMenuItem(startY + lastMenuSelection * MENU_ITEM_H, tempItems[lastMenuSelection], false);
        }
        if (menuSelection < 5) {
            drawMenuItem(startY + menuSelection * MENU_ITEM_H, tempItems[menuSelection], true);
        }
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
    
    // Setting items with dynamic values
    static char clockLabel[32];
    static char multitaskLabel[32];
    snprintf(clockLabel, sizeof(clockLabel), "Clock: %.0f BPM", clockTempo);
    snprintf(multitaskLabel, sizeof(multitaskLabel), "Multitask: %s", multitaskingMode ? "ON" : "OFF");
    
    const char* settings[] = {
        clockLabel,
        multitaskLabel,
        "Audio Output",
        "MIDI Channel",
        "Display Brightness",
        "Script Auto-load"
    };
    
    menuItemCount = 6;
    
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
    display->drawText(MARGIN, 50, "Polyphonion", COLOR_FG, FONT_MEDIUM);
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
    
    // Reset sequencer and LFO drawing state to force full redraw
    for (int i = 0; i < MAX_SCRIPTS; i++) {
        scriptSlots[i].lastSeqCurrentStep = 255;
        scriptSlots[i].lastSeqEditStep = 255;
        scriptSlots[i].lastSeqEditMode = 255;
        scriptSlots[i].lastLfoWaveType = 255;
        scriptSlots[i].lastLfoFrequency = -1.0f;
        scriptSlots[i].lastLfoLevel = -1.0f;
        scriptSlots[i].lastLfoEditParam = 255;
    }
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
    // Handle settings based on current selection
    if (menuSelection == 0) {
        // Clock tempo - controlled by encoder in main.cpp
        showSettingsScreen();
    } else if (menuSelection == 1) {
        // Toggle multitasking mode
        toggleMultitaskingMode();
        showSettingsScreen();
    }
}

void UI::setClockTempo(float bpm) {
    if (bpm < MIN_CLOCK_BPM) bpm = MIN_CLOCK_BPM;
    if (bpm > MAX_CLOCK_BPM) bpm = MAX_CLOCK_BPM;
    clockTempo = bpm;
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
    
    // Calculate position based on multitasking mode
    int16_t x, y, w, h;
    if (multitaskingMode) {
        // Quadrant mode (future)
        x = (slot % 2) * (SCREEN_WIDTH / 2);
        y = (slot / 2) * (SCREEN_HEIGHT / 2);
        w = SCREEN_WIDTH / 2 - 2;
        h = SCREEN_HEIGHT / 2 - 2;
    } else {
        // Full screen mode
        x = 0;
        y = 0;
        w = SCREEN_WIDTH;
        h = SCREEN_HEIGHT;
    }
    
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
        if (scriptSlots[slot].scriptType == 2) {
            // Steampunk title at top
            display->drawText(x + 20, y + 5, "STEAMPUNQUENCER", COLOR_FG, FONT_SMALL);
        } else {
            display->drawText(x + 20, y + 5, scriptSlots[slot].name, COLOR_FG, FONT_MEDIUM);
        }
        
        // Draw running indicator (cyan dot)
        display->fillCircle(x + w - 10, y + 10, 4, COLOR_ACCENT);
        
        // Debug: Show script type
        char debugType[16];
        snprintf(debugType, sizeof(debugType), "T:%d", scriptSlots[slot].scriptType);
        display->drawText(x + 5, y + 18, debugType, COLOR_DIM, FONT_SMALL);
        
        // Render based on script type
        if (scriptSlots[slot].scriptType == 0) {
            // LFO - draw waveform visualization (70%) + editable parameters (30%)
            int16_t contentY = y + 30;
            int16_t contentH = h - 35;
            
            // Waveform visualization area (70% of content height)
            int16_t waveH = (contentH * 70) / 100;
            int16_t waveX = x + 8;
            int16_t waveY = contentY;
            int16_t waveW = w - 16;
            int16_t waveCenterY = waveY + waveH / 2;
            
            // Always redraw waveform for animation
            display->fillRect(waveX, waveY, waveW, waveH, COLOR_BG);
            display->drawLine(waveX, waveCenterY, waveX + waveW, waveCenterY, COLOR_DIM);
            
            uint8_t waveType = scriptSlots[slot].waveType;
            float phase = scriptSlots[slot].phase;
            
            for (int16_t i = 0; i < waveW; i++) {
                float t = (float)i / waveW * 2.0f * PI + phase;
                float value = 0.0f;
                
                switch (waveType) {
                    case 0: value = sin(t); break;
                    case 1: {
                        float normalized = fmod(t, 2.0f * PI) / (2.0f * PI);
                        value = (normalized < 0.5f) ? (normalized * 4.0f - 1.0f) : (3.0f - normalized * 4.0f);
                        break;
                    }
                    case 2: value = (fmod(t, 2.0f * PI) < PI) ? 1.0f : -1.0f; break;
                    case 3: value = 2.0f * (fmod(t, 2.0f * PI) / (2.0f * PI)) - 1.0f; break;
                }
                
                int16_t pixelY = waveCenterY - (int16_t)(value * waveH * 0.4f);
                display->drawPixel(waveX + i, pixelY, COLOR_ACCENT);
            }
            
            // Parameters area (30% at bottom)
            int16_t paramsY = waveY + waveH + 5;
            const char* waveNames[] = {"SINE", "TRI", "SQR", "SAW"};
            
            // Only redraw parameters if changed or edit param changed
            if (scriptSlots[slot].waveType != scriptSlots[slot].lastLfoWaveType ||
                scriptSlots[slot].lfoEditParam != scriptSlots[slot].lastLfoEditParam) {
                
                // Clear old highlight if was editing waveform
                if (scriptSlots[slot].lastLfoEditParam == 0) {
                    display->drawRect(x + 5, paramsY - 2, 150, 14, COLOR_BG);
                    display->drawRect(x + 6, paramsY - 1, 148, 12, COLOR_BG);
                }
                
                // Clear old value area
                display->fillRect(x + 8, paramsY, 145, 10, COLOR_BG);
                
                // Draw waveform name
                char waveStr[32];
                snprintf(waveStr, sizeof(waveStr), "Wave: %s", waveNames[scriptSlots[slot].waveType]);
                display->drawText(x + 8, paramsY, waveStr, COLOR_FG, FONT_SMALL);
                
                // Draw highlight box if editing
                if (scriptSlots[slot].lfoEditParam == 0) {
                    display->drawRect(x + 5, paramsY - 2, 150, 14, COLOR_HIGHLIGHT);
                    display->drawRect(x + 6, paramsY - 1, 148, 12, COLOR_HIGHLIGHT);
                }
                
                scriptSlots[slot].lastLfoWaveType = scriptSlots[slot].waveType;
            }
            
            // Frequency
            int16_t freqY = paramsY + 12;
            
            if (scriptSlots[slot].lfoFrequency != scriptSlots[slot].lastLfoFrequency ||
                scriptSlots[slot].lfoEditParam != scriptSlots[slot].lastLfoEditParam) {
                
                // Clear old highlight if was editing frequency
                if (scriptSlots[slot].lastLfoEditParam == 1) {
                    display->drawRect(x + 5, freqY - 2, 150, 14, COLOR_BG);
                    display->drawRect(x + 6, freqY - 1, 148, 12, COLOR_BG);
                }
                
                // Clear old value area
                display->fillRect(x + 8, freqY, 145, 10, COLOR_BG);
                
                // Draw frequency value
                char freqStr[32];
                if (scriptSlots[slot].lfoFrequency < 1.0f) {
                    snprintf(freqStr, sizeof(freqStr), "Freq: %.2f Hz", scriptSlots[slot].lfoFrequency);
                } else if (scriptSlots[slot].lfoFrequency < 10.0f) {
                    snprintf(freqStr, sizeof(freqStr), "Freq: %.1f Hz", scriptSlots[slot].lfoFrequency);
                } else {
                    snprintf(freqStr, sizeof(freqStr), "Freq: %.0f Hz", scriptSlots[slot].lfoFrequency);
                }
                display->drawText(x + 8, freqY, freqStr, COLOR_FG, FONT_SMALL);
                
                // Draw highlight box if editing
                if (scriptSlots[slot].lfoEditParam == 1) {
                    display->drawRect(x + 5, freqY - 2, 150, 14, COLOR_HIGHLIGHT);
                    display->drawRect(x + 6, freqY - 1, 148, 12, COLOR_HIGHLIGHT);
                }
                
                scriptSlots[slot].lastLfoFrequency = scriptSlots[slot].lfoFrequency;
            }
            
            // Level
            int16_t levelY = freqY + 12;
            
            if (scriptSlots[slot].lfoLevel != scriptSlots[slot].lastLfoLevel ||
                scriptSlots[slot].lfoEditParam != scriptSlots[slot].lastLfoEditParam) {
                
                // Clear old highlight if was editing level
                if (scriptSlots[slot].lastLfoEditParam == 2) {
                    display->drawRect(x + 5, levelY - 2, 150, 14, COLOR_BG);
                    display->drawRect(x + 6, levelY - 1, 148, 12, COLOR_BG);
                }
                
                // Clear old value area
                display->fillRect(x + 8, levelY, 145, 10, COLOR_BG);
                
                // Draw level value
                char levelStr[32];
                snprintf(levelStr, sizeof(levelStr), "Level: %.1f V", scriptSlots[slot].lfoLevel);
                display->drawText(x + 8, levelY, levelStr, COLOR_FG, FONT_SMALL);
                
                // Draw highlight box if editing
                if (scriptSlots[slot].lfoEditParam == 2) {
                    display->drawRect(x + 5, levelY - 2, 150, 14, COLOR_HIGHLIGHT);
                    display->drawRect(x + 6, levelY - 1, 148, 12, COLOR_HIGHLIGHT);
                }
                
                scriptSlots[slot].lastLfoLevel = scriptSlots[slot].lfoLevel;
            }
            
            // Update last edit param
            scriptSlots[slot].lastLfoEditParam = scriptSlots[slot].lfoEditParam;
        } else if (scriptSlots[slot].scriptType == 1) {
            // Sequencer - split screen: top 3/4 for sliders, bottom 1/4 for dials
            int16_t contentY = y + 25;
            int16_t contentH = h - 30;
            int16_t sliderH = (contentH * 3) / 4;
            int16_t dialH = contentH - sliderH;
            
            drawSequencerSliders(slot, x, contentY, w, sliderH);
            drawSequencerDials(slot, x, contentY + sliderH, w, dialH);
        } else if (scriptSlots[slot].scriptType == 2) {
            // SteampunQuencer - full steampunk aesthetic
            int16_t contentY = y + 25;
            int16_t contentH = h - 30;
            drawSteampunkSequencer(slot, x, contentY, w, contentH);
        } else {
            // Unknown script type - show error
            display->fillRect(x + 6, y + 25, w - 12, h - 30, COLOR_BG);
            display->drawText(x + 8, y + 40, "Unknown type", COLOR_DIM, FONT_SMALL);
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

void UI::drawSequencerSliders(uint8_t slot, int16_t x, int16_t y, int16_t w, int16_t h) {
    if (!display || slot >= MAX_SCRIPTS) return;
    
    // Check if this is first draw or if parameters changed significantly
    bool firstDraw = (scriptSlots[slot].lastSeqCurrentStep == 255);  // 255 = uninitialized
    bool editingDuration = scriptSlots[slot].seqEditingDuration;
    bool lastEditingDuration = scriptSlots[slot].lastSeqEditingDuration;
    
    // Calculate slider dimensions
    int16_t sliderSpacing = 2;
    int16_t sliderWidth = (w - 20) / 8 - sliderSpacing;
    int16_t sliderHeight = h - 10;
    int16_t sliderStartX = x + 10;
    int16_t sliderStartY = y + 5;
    
    uint8_t currentStep = scriptSlots[slot].seqCurrentStep;
    uint8_t editStep = scriptSlots[slot].seqEditStep;
    uint8_t lastCurrentStep = scriptSlots[slot].lastSeqCurrentStep;
    uint8_t lastEditStep = scriptSlots[slot].lastSeqEditStep;
    
    if (firstDraw) {
        // First draw - clear everything and draw all sliders
        display->fillRect(x + 4, y, w - 8, h, COLOR_BG);
        
        // Draw all 8 sliders
        for (int i = 0; i < 8; i++) {
            int16_t sliderX = sliderStartX + i * (sliderWidth + sliderSpacing);
            int8_t stepValue = scriptSlots[slot].seqStepValues[i];
            
            // Draw slider background
            display->drawRect(sliderX, sliderStartY, sliderWidth, sliderHeight, COLOR_DIM);
            
            // Calculate fill height
            float normalizedValue = (float)(stepValue + 12) / 24.0f;
            if (normalizedValue < 0.0f) normalizedValue = 0.0f;
            if (normalizedValue > 1.0f) normalizedValue = 1.0f;
            
            int16_t fillHeight = (int16_t)(normalizedValue * (sliderHeight - 2));
            int16_t fillY = sliderStartY + sliderHeight - 2 - fillHeight;
            
            // Choose color based on state
            uint16_t fillColor = COLOR_DIM;
            if (i == currentStep) {
                fillColor = COLOR_ACCENT;
            } else if (i == editStep && !editingDuration) {
                fillColor = COLOR_HIGHLIGHT;  // Highlight only if editing pitch
            }
            
            // Draw filled portion
            if (fillHeight > 0) {
                display->fillRect(sliderX + 1, fillY, sliderWidth - 2, fillHeight, fillColor);
            }
        }
    } else {
        // Selective update - only redraw changed sliders
        for (int i = 0; i < 8; i++) {
            int8_t stepValue = scriptSlots[slot].seqStepValues[i];
            int8_t lastStepValue = scriptSlots[slot].lastSeqStepValues[i];
            
            // Check if this slider needs updating
            bool needsUpdate = false;
            if (stepValue != lastStepValue) needsUpdate = true;
            if (i == currentStep && i != lastCurrentStep) needsUpdate = true;
            if (i == lastCurrentStep && i != currentStep) needsUpdate = true;
            if (i == editStep && (i != lastEditStep || editingDuration != lastEditingDuration)) needsUpdate = true;
            if (i == lastEditStep && (i != editStep || editingDuration != lastEditingDuration)) needsUpdate = true;
            
            if (needsUpdate) {
                int16_t sliderX = sliderStartX + i * (sliderWidth + sliderSpacing);
                
                // Clear the slider interior
                display->fillRect(sliderX + 1, sliderStartY + 1, sliderWidth - 2, sliderHeight - 2, COLOR_BG);
                
                // Calculate fill height
                float normalizedValue = (float)(stepValue + 12) / 24.0f;
                if (normalizedValue < 0.0f) normalizedValue = 0.0f;
                if (normalizedValue > 1.0f) normalizedValue = 1.0f;
                
                int16_t fillHeight = (int16_t)(normalizedValue * (sliderHeight - 2));
                int16_t fillY = sliderStartY + sliderHeight - 2 - fillHeight;
                
                // Choose color based on state
                uint16_t fillColor = COLOR_DIM;
                if (i == currentStep) {
                    fillColor = COLOR_ACCENT;
                } else if (i == editStep && !editingDuration) {
                    fillColor = COLOR_HIGHLIGHT;  // Highlight only if editing pitch
                }
                
                // Draw filled portion
                if (fillHeight > 0) {
                    display->fillRect(sliderX + 1, fillY, sliderWidth - 2, fillHeight, fillColor);
                }
            }
        }
    }
    
    // Update tracking state
    scriptSlots[slot].lastSeqCurrentStep = currentStep;
    scriptSlots[slot].lastSeqEditStep = editStep;
    scriptSlots[slot].lastSeqEditingDuration = editingDuration;
    for (int i = 0; i < 8; i++) {
        scriptSlots[slot].lastSeqStepValues[i] = scriptSlots[slot].seqStepValues[i];
    }
}

void UI::drawSequencerDials(uint8_t slot, int16_t x, int16_t y, int16_t w, int16_t h) {
    if (!display || slot >= MAX_SCRIPTS) return;
    
    bool firstDraw = (scriptSlots[slot].lastSeqCurrentStep == 255);
    bool editingDuration = scriptSlots[slot].seqEditingDuration;
    bool lastEditingDuration = scriptSlots[slot].lastSeqEditingDuration;
    
    // Force redraw of all dials if any duration value hasn't been initialized
    if (!firstDraw) {
        for (int i = 0; i < 8; i++) {
            if (scriptSlots[slot].lastSeqStepDurations[i] == 0) {
                firstDraw = true;
                break;
            }
        }
    }
    
    // Calculate dial dimensions
    int16_t dialSpacing = 2;
    int16_t dialWidth = (w - 20) / 8 - dialSpacing;
    int16_t dialRadius = (dialWidth > (h - 10)) ? (h - 10) / 2 : dialWidth / 2;
    if (dialRadius < 8) dialRadius = 8;
    int16_t dialStartX = x + 10 + dialWidth / 2;
    int16_t dialCenterY = y + h / 2;
    
    uint8_t currentStep = scriptSlots[slot].seqCurrentStep;
    uint8_t editStep = scriptSlots[slot].seqEditStep;
    uint8_t lastEditStep = scriptSlots[slot].lastSeqEditStep;
    
    if (firstDraw) {
        // First draw - clear and draw all dials
        display->fillRect(x + 4, y, w - 8, h, COLOR_BG);
        
        for (int i = 0; i < 8; i++) {
            int16_t dialCenterX = dialStartX + i * (dialWidth + dialSpacing);
            uint8_t duration = scriptSlots[slot].seqStepDurations[i];
            
            // Draw potentiometer-style dial
            // Outer circle - draw twice for brightness in light blue
            display->drawCircle(dialCenterX, dialCenterY, dialRadius, COLOR_DIAL);
            display->drawCircle(dialCenterX, dialCenterY, dialRadius - 1, COLOR_DIAL);
            
            // Draw tick marks for positions 1-8 around the circle
            for (int pos = 1; pos <= 8; pos++) {
                // Calculate angle: start at 8 o'clock (150°), sweep clockwise to 4 o'clock (30°)
                // That's 240° total sweep for 8 positions
                float angle = (150.0f + ((pos - 1) * 240.0f / 7.0f)) * PI / 180.0f;
                int16_t tickX = dialCenterX + (int16_t)((dialRadius - 2) * cos(angle));
                int16_t tickY = dialCenterY + (int16_t)((dialRadius - 2) * sin(angle));
                // Draw tick marks in light blue
                display->fillCircle(tickX, tickY, 1, COLOR_DIAL);
            }
            
            // Draw pointer indicating current value
            float pointerAngle = (150.0f + ((duration - 1) * 240.0f / 7.0f)) * PI / 180.0f;
            int16_t pointerX = dialCenterX + (int16_t)((dialRadius - 3) * cos(pointerAngle));
            int16_t pointerY = dialCenterY + (int16_t)((dialRadius - 3) * sin(pointerAngle));
            
            uint16_t pointerColor = COLOR_DIAL;
            if (i == editStep && editingDuration) {
                pointerColor = COLOR_HIGHLIGHT;
            } else if (i == currentStep) {
                pointerColor = COLOR_ACCENT;
            }
            
            // Draw pointer line from center - draw twice for brightness
            display->drawLine(dialCenterX, dialCenterY, pointerX, pointerY, pointerColor);
            display->drawLine(dialCenterX + 1, dialCenterY, pointerX + 1, pointerY, pointerColor);
            
            // Draw larger circle at pointer tip for visibility
            display->fillCircle(pointerX, pointerY, 2, pointerColor);
        }
    } else {
        // Selective update - only redraw changed dials
        for (int i = 0; i < 8; i++) {
            uint8_t duration = scriptSlots[slot].seqStepDurations[i];
            uint8_t lastDuration = scriptSlots[slot].lastSeqStepDurations[i];
            
            bool needsUpdate = false;
            if (duration != lastDuration) needsUpdate = true;
            if (i == editStep && (i != lastEditStep || editingDuration != lastEditingDuration)) needsUpdate = true;
            if (i == lastEditStep && (i != editStep || editingDuration != lastEditingDuration)) needsUpdate = true;
            
            if (needsUpdate) {
                int16_t dialCenterX = dialStartX + i * (dialWidth + dialSpacing);
                
                // Clear dial interior (but not the outer ring or tick marks)
                display->fillCircle(dialCenterX, dialCenterY, dialRadius - 3, COLOR_BG);
                
                // Redraw pointer for new value
                float pointerAngle = (150.0f + ((duration - 1) * 240.0f / 7.0f)) * PI / 180.0f;
                int16_t pointerX = dialCenterX + (int16_t)((dialRadius - 3) * cos(pointerAngle));
                int16_t pointerY = dialCenterY + (int16_t)((dialRadius - 3) * sin(pointerAngle));
                
                uint16_t pointerColor = COLOR_DIAL;
                if (i == editStep && editingDuration) {
                    pointerColor = COLOR_HIGHLIGHT;
                } else if (i == currentStep) {
                    pointerColor = COLOR_ACCENT;
                }
                
                // Draw pointer line from center - draw twice for brightness
                display->drawLine(dialCenterX, dialCenterY, pointerX, pointerY, pointerColor);
                display->drawLine(dialCenterX + 1, dialCenterY, pointerX + 1, pointerY, pointerColor);
                
                // Draw larger circle at pointer tip for visibility
                display->fillCircle(pointerX, pointerY, 2, pointerColor);
            }
        }
    }
    
    // Update tracking state
    for (int i = 0; i < 8; i++) {
        scriptSlots[slot].lastSeqStepDurations[i] = scriptSlots[slot].seqStepDurations[i];
    }
}

void UI::setScriptType(uint8_t slot, uint8_t type) {
    if (slot < MAX_SCRIPTS) {
        scriptSlots[slot].scriptType = type;
    }
}

void UI::updateScriptSequencer(uint8_t slot, uint8_t currentStep, int8_t stepValues[8], uint8_t stepDurations[8]) {
    if (slot >= MAX_SCRIPTS) return;
    
    scriptSlots[slot].seqCurrentStep = currentStep;
    if (stepValues) {
        for (int i = 0; i < 8; i++) {
            scriptSlots[slot].seqStepValues[i] = stepValues[i];
        }
    }
    if (stepDurations) {
        for (int i = 0; i < 8; i++) {
            scriptSlots[slot].seqStepDurations[i] = stepDurations[i];
        }
    }
}

void UI::advanceSequencerEditStep(uint8_t slot) {
    if (slot < MAX_SCRIPTS) {
        // Check if this is a steampunk sequencer (has gate modes)
        bool isSteampunk = (scriptSlots[slot].scriptType == 2);
        
        if (isSteampunk) {
            // Steampunk: cycle through lever → switch → crank → next step's lever
            scriptSlots[slot].seqEditMode++;
            if (scriptSlots[slot].seqEditMode > 2) {
                scriptSlots[slot].seqEditMode = 0;
                scriptSlots[slot].seqEditStep = (scriptSlots[slot].seqEditStep + 1) % 8;
            }
            // Update legacy duration flag for compatibility
            scriptSlots[slot].seqEditingDuration = (scriptSlots[slot].seqEditMode == 2);
        } else {
            // Regular sequencer: toggle between pitch and duration
            if (!scriptSlots[slot].seqEditingDuration) {
                scriptSlots[slot].seqEditingDuration = true;
                scriptSlots[slot].seqEditMode = 2;
            } else {
                scriptSlots[slot].seqEditingDuration = false;
                scriptSlots[slot].seqEditMode = 0;
                scriptSlots[slot].seqEditStep = (scriptSlots[slot].seqEditStep + 1) % 8;
            }
        }
    }
}

void UI::toggleSequencerEditMode(uint8_t slot) {
    if (slot < MAX_SCRIPTS) {
        scriptSlots[slot].seqEditingDuration = !scriptSlots[slot].seqEditingDuration;
    }
}

void UI::adjustSequencerStepValue(uint8_t slot, int8_t delta) {
    if (slot >= MAX_SCRIPTS) return;
    
    uint8_t editStep = scriptSlots[slot].seqEditStep;
    scriptSlots[slot].seqStepValues[editStep] += delta;
    
    // Clamp to valid range (-12 to +12)
    if (scriptSlots[slot].seqStepValues[editStep] < -12) {
        scriptSlots[slot].seqStepValues[editStep] = -12;
    }
    if (scriptSlots[slot].seqStepValues[editStep] > 12) {
        scriptSlots[slot].seqStepValues[editStep] = 12;
    }
}

void UI::adjustSequencerStepDuration(uint8_t slot, int8_t delta) {
    if (slot >= MAX_SCRIPTS) return;
    
    uint8_t editStep = scriptSlots[slot].seqEditStep;
    scriptSlots[slot].seqStepDurations[editStep] += delta;
    
    // Clamp to valid range (1 to 8)
    if (scriptSlots[slot].seqStepDurations[editStep] < 1) {
        scriptSlots[slot].seqStepDurations[editStep] = 1;
    }
    if (scriptSlots[slot].seqStepDurations[editStep] > 8) {
        scriptSlots[slot].seqStepDurations[editStep] = 8;
    }
}

// ========== LFO EDITING ==========

void UI::advanceLFOEditParam(uint8_t slot) {
    if (slot >= MAX_SCRIPTS) return;
    scriptSlots[slot].lfoEditParam = (scriptSlots[slot].lfoEditParam + 1) % 3;
}

void UI::adjustLFOWaveform(uint8_t slot, int delta) {
    if (slot >= MAX_SCRIPTS) return;
    
    int8_t waveType = scriptSlots[slot].waveType;
    waveType += delta;
    
    // Wrap around 0-3
    if (waveType < 0) waveType = 3;
    if (waveType > 3) waveType = 0;
    
    scriptSlots[slot].waveType = waveType;
}

void UI::adjustLFOFrequency(uint8_t slot, int delta) {
    if (slot >= MAX_SCRIPTS) return;
    
    // Logarithmic adjustment for frequency
    float step = 0.1f;
    if (scriptSlots[slot].lfoFrequency >= 10.0f) step = 1.0f;
    else if (scriptSlots[slot].lfoFrequency >= 1.0f) step = 0.5f;
    
    scriptSlots[slot].lfoFrequency += (delta * step);
    
    // Clamp to valid range (0.01 to 100 Hz)
    if (scriptSlots[slot].lfoFrequency < 0.01f) {
        scriptSlots[slot].lfoFrequency = 0.01f;
    }
    if (scriptSlots[slot].lfoFrequency > 100.0f) {
        scriptSlots[slot].lfoFrequency = 100.0f;
    }
}

void UI::adjustLFOLevel(uint8_t slot, int delta) {
    if (slot >= MAX_SCRIPTS) return;
    
    scriptSlots[slot].lfoLevel += (delta * 0.1f);
    
    // Clamp to valid range (0.0 to 5.0 V)
    if (scriptSlots[slot].lfoLevel < 0.0f) {
        scriptSlots[slot].lfoLevel = 0.0f;
    }
    if (scriptSlots[slot].lfoLevel > 5.0f) {
        scriptSlots[slot].lfoLevel = 5.0f;
    }
}

// ========== STEAMPUNK SEQUENCER ==========

void UI::updateSteampunkSequencer(uint8_t slot, uint8_t currentStep, uint8_t currentBeat, int8_t stepValues[8], uint8_t stepDurations[8], uint8_t gateModes[8], uint8_t direction, bool steamTrigger) {
    if (slot >= MAX_SCRIPTS) return;
    
    scriptSlots[slot].seqCurrentStep = currentStep;
    scriptSlots[slot].seqCurrentBeat = currentBeat;
    scriptSlots[slot].seqDirection = direction;
    scriptSlots[slot].seqSteamTrigger = steamTrigger;
    
    for (int i = 0; i < 8; i++) {
        scriptSlots[slot].seqStepValues[i] = stepValues[i];
        scriptSlots[slot].seqStepDurations[i] = stepDurations[i];
        scriptSlots[slot].seqGateModes[i] = gateModes[i];
    }
}

void UI::toggleStepGateMode(uint8_t slot) {
    if (slot >= MAX_SCRIPTS) return;
    
    uint8_t editStep = scriptSlots[slot].seqEditStep;
    scriptSlots[slot].seqGateModes[editStep] = (scriptSlots[slot].seqGateModes[editStep] + 1) % 3;
}

void UI::adjustStepGateMode(uint8_t slot, int8_t delta) {
    if (slot >= MAX_SCRIPTS) return;
    
    uint8_t editStep = scriptSlots[slot].seqEditStep;
    int8_t newMode = (int8_t)scriptSlots[slot].seqGateModes[editStep] + delta;
    // Wrap around 0-2
    if (newMode < 0) newMode = 2;
    if (newMode > 2) newMode = 0;
    scriptSlots[slot].seqGateModes[editStep] = (uint8_t)newMode;
}

void UI::cycleDirection(uint8_t slot) {
    if (slot >= MAX_SCRIPTS) return;
    
    scriptSlots[slot].seqDirection = (scriptSlots[slot].seqDirection + 1) % 4;
}

void UI::drawSteampunkSequencer(uint8_t slot, int16_t x, int16_t y, int16_t w, int16_t h) {
    if (!display || slot >= MAX_SCRIPTS) return;
    
    bool firstDraw = (scriptSlots[slot].lastSeqCurrentStep == 255);
    
    // Layout: 
    // - Top 10px: Brass nameplate with title
    // - Next 110px: Levers (pitch control)
    // - Next 30px: Toggle switches (gate mode)
    // - Bottom 60px: Hand cranks (duration/ratchets)
    
    int16_t leverY = y;
    int16_t leverH = 115;
    int16_t switchY = leverY + leverH + 5;
    int16_t switchH = 30;
    int16_t crankY = switchY + switchH + 5;
    int16_t crankH = h - (crankY - y);
    
    uint8_t currentStep = scriptSlots[slot].seqCurrentStep;
    uint8_t editStep = scriptSlots[slot].seqEditStep;
    
    if (firstDraw) {
        display->fillRect(x + 4, y, w - 8, h, COLOR_BG);
        
        // === BRASS NAMEPLATE (decorative only) ===
        // Corner rivets
        display->fillCircle(x + 5, y + 5, 2, COLOR_DIM);
        display->fillCircle(x + w - 10, y + 5, 2, COLOR_DIM);
    }
    
    // Direction indicator (top right corner) - update only when changed
    if (firstDraw || scriptSlots[slot].seqDirection != scriptSlots[slot].lastSeqDirection) {
        // Clear old direction
        display->fillRect(x + w - 35, y, 25, 10, COLOR_BG);
        // Draw new direction
        const char* dirSymbols[] = {">>>", "<<<", "<>", "???"};
        display->drawText(x + w - 35, y, dirSymbols[scriptSlots[slot].seqDirection], COLOR_ACCENT, FONT_SMALL);
        scriptSlots[slot].lastSeqDirection = scriptSlots[slot].seqDirection;
    }
    
    // === LEVERS (Pitch Control) ===
    int16_t leverSpacing = 2;
    int16_t leverWidth = (w - 30) / 8 - leverSpacing;
    int16_t leverStartX = x + 15;
    
    for (int i = 0; i < 8; i++) {
        int16_t leverX = leverStartX + i * (leverWidth + leverSpacing);
        int8_t stepValue = scriptSlots[slot].seqStepValues[i];
        int8_t lastStepValue = scriptSlots[slot].lastSeqStepValues[i];
        
        uint8_t editMode = scriptSlots[slot].seqEditMode;
        uint8_t lastEditMode = scriptSlots[slot].lastSeqEditMode;
        bool isEditingThisStep = (i == editStep && editMode == 0);
        
        // Only redraw if value changed or edit step changed
        bool needsUpdate = firstDraw;
        if (stepValue != lastStepValue) needsUpdate = true;
        if (i == editStep && scriptSlots[slot].lastSeqEditStep != editStep && editMode == 0) needsUpdate = true;
        if (i == scriptSlots[slot].lastSeqEditStep && editStep != i) needsUpdate = true;
        // Redraw when edit mode changes for this step to clear/add highlight box
        if (i == editStep && editMode != lastEditMode) needsUpdate = true;
        
        // Separate check for handle color change (currentStep trigger)
        // Don't flash if we're editing this step - keep it stable
        bool handleColorChanged = false;
        if (!isEditingThisStep) {
            if (i == currentStep && scriptSlots[slot].lastSeqCurrentStep != currentStep) handleColorChanged = true;
            if (i == scriptSlots[slot].lastSeqCurrentStep && i != currentStep) handleColorChanged = true;
        }
        
        if (needsUpdate || handleColorChanged) {
            // Calculate lever position
            float normalizedValue = (float)(stepValue + 12) / 24.0f;
            if (normalizedValue < 0.0f) normalizedValue = 0.0f;
            if (normalizedValue > 1.0f) normalizedValue = 1.0f;
            
            int16_t handleHeight = 10;
            int16_t shaftHeight = leverH - handleHeight - 5;
            int16_t handleY = leverY + (int16_t)((1.0f - normalizedValue) * shaftHeight);
            
            if (needsUpdate) {
                // Full redraw: clear lever area
                display->fillRect(leverX, leverY, leverWidth, leverH, COLOR_BG);
                
                // Lever shaft (double line for depth)
                int16_t shaftX = leverX + leverWidth / 2;
                display->drawLine(shaftX - 1, leverY, shaftX - 1, leverY + shaftHeight, COLOR_DIM);
                display->drawLine(shaftX, leverY, shaftX, leverY + shaftHeight, COLOR_FG);
            } else if (handleColorChanged) {
                // Only redraw handle for trigger flash
                display->fillRect(leverX, handleY - 2, leverWidth, handleHeight + 4, COLOR_BG);
            }
            
            // Lever handle (beveled rectangle)
            uint16_t handleColor = COLOR_DIM;
            
            // Priority: edit mode > trigger flash > current step
            if (isEditingThisStep) {
                // Light gray for editing - stable, no flicker
                handleColor = 0x7BEF;  // Light gray (between DIM and FG)
            } else if (i == currentStep && scriptSlots[slot].seqSteamTrigger) {
                // RED FLASH when triggered (currentStep)
                handleColor = 0xF800;  // Red for trigger flash
            } else if (i == currentStep) {
                // Normal current step indicator
                handleColor = COLOR_ACCENT;
            }
            
            display->fillRect(leverX + 1, handleY, leverWidth - 2, handleHeight, handleColor);
            // Highlight edge
            display->drawLine(leverX + 2, handleY + 1, leverX + leverWidth - 3, handleY + 1, COLOR_FG);
        }
        
        // Draw highlight box around entire lever ONLY when in lever edit mode (mode 0)
        // Draw after loop to overlay on top
        if (i == editStep && editMode == 0) {
            display->drawRect(leverX, leverY, leverWidth, leverH, COLOR_HIGHLIGHT);
            display->drawRect(leverX + 1, leverY + 1, leverWidth - 2, leverH - 2, COLOR_HIGHLIGHT);
        }
    }
    
    // === TOGGLE SWITCHES (Gate Mode) ===
    int16_t switchWidth = leverWidth;
    
    for (int i = 0; i < 8; i++) {
        int16_t switchX = leverStartX + i * (leverWidth + leverSpacing);
        uint8_t gateMode = scriptSlots[slot].seqGateModes[i];
        uint8_t lastGateMode = scriptSlots[slot].lastSeqGateModes[i];
        
        uint8_t editMode = scriptSlots[slot].seqEditMode;
        uint8_t lastEditMode = scriptSlots[slot].lastSeqEditMode;
        
        // Only redraw if gate mode changed or edit highlighting changed
        bool needsUpdate = firstDraw || (gateMode != lastGateMode);
        // Only update on edit step transitions or mode changes when this step is involved
        if ((i == editStep || i == scriptSlots[slot].lastSeqEditStep) && editMode != lastEditMode) needsUpdate = true;
        if (i == editStep && scriptSlots[slot].lastSeqEditStep != editStep && editMode == 1) needsUpdate = true;
        if (i == scriptSlots[slot].lastSeqEditStep && editStep != i && lastEditMode == 1) needsUpdate = true;
        
        if (needsUpdate) {
            // Clear switch area
            display->fillRect(switchX, switchY, switchWidth, switchH, COLOR_BG);
            
            // Base plate (larger, more visible)
            display->fillRect(switchX + 1, switchY + 2, switchWidth - 2, switchH - 4, COLOR_DIM);
            
            // Draw switch state with much larger, clearer graphics
            if (gateMode == 0) {
                // NORMAL: Filled rectangle (ON position - top)
                display->fillRect(switchX + 2, switchY + 4, switchWidth - 4, 8, COLOR_FG);
                display->fillRect(switchX + 2, switchY + 5, switchWidth - 4, 6, COLOR_ACCENT);
            } else if (gateMode == 1) {
                // SKIP: Empty rectangle (OFF position - bottom)
                display->drawRect(switchX + 2, switchY + switchH - 12, switchWidth - 4, 8, COLOR_FG);
                // Draw X inside to indicate skip
                display->drawLine(switchX + 3, switchY + switchH - 11, switchX + switchWidth - 3, switchY + switchH - 5, COLOR_DIM);
                display->drawLine(switchX + switchWidth - 3, switchY + switchH - 11, switchX + 3, switchY + switchH - 5, COLOR_DIM);
            } else if (gateMode == 2) {
                // SLIDE: Filled rectangle in middle position (glide/portamento)
                int16_t midY = switchY + switchH / 2;
                display->fillRect(switchX + 2, midY - 4, switchWidth - 4, 8, COLOR_FG);
                display->fillRect(switchX + 2, midY - 3, switchWidth - 4, 6, COLOR_DIAL);
            }
            
            // Highlight border if editing switch (mode 1)
            if (i == editStep && editMode == 1) {
                display->drawRect(switchX, switchY, switchWidth, switchH, COLOR_HIGHLIGHT);
                display->drawRect(switchX + 1, switchY + 1, switchWidth - 2, switchH - 2, COLOR_HIGHLIGHT);
            }
        }
    }
    
    // === HAND CRANKS (Duration/Ratchets) ===
    int16_t crankCenterY = crankY + 20;
    int16_t crankRadius = 8;
    
    for (int i = 0; i < 8; i++) {
        int16_t crankCenterX = leverStartX + leverWidth / 2 + i * (leverWidth + leverSpacing);
        uint8_t duration = scriptSlots[slot].seqStepDurations[i];
        uint8_t lastDuration = scriptSlots[slot].lastSeqStepDurations[i];
        
        uint8_t editMode = scriptSlots[slot].seqEditMode;
        uint8_t lastEditMode = scriptSlots[slot].lastSeqEditMode;
        
        // Only redraw if duration changed or edit highlighting changed
        bool needsUpdate = firstDraw || (duration != lastDuration);
        // Only update on edit step transitions when in crank edit mode
        if ((i == editStep || i == scriptSlots[slot].lastSeqEditStep) && editMode != lastEditMode) needsUpdate = true;
        if (i == editStep && scriptSlots[slot].lastSeqEditStep != editStep && editMode == 2) needsUpdate = true;
        if (i == scriptSlots[slot].lastSeqEditStep && editStep != i && lastEditMode == 2) needsUpdate = true;
        
        if (needsUpdate) {
            // Clear crank area (wider to include potential highlight box)
            int16_t clearWidth = crankRadius * 2 + 10;  // Extra space for highlight box
            display->fillRect(crankCenterX - clearWidth/2, crankY, clearWidth, crankH, COLOR_BG);
            
            // Outer rim (copper wheel) - double drawn in light blue
            display->drawCircle(crankCenterX, crankCenterY, crankRadius, COLOR_DIAL);
            display->drawCircle(crankCenterX, crankCenterY, crankRadius - 1, COLOR_DIAL);
            
            // Spokes (4 lines)
            for (int s = 0; s < 4; s++) {
                float spokeAngle = (s * 90.0f) * PI / 180.0f;
                int16_t spokeEndX = crankCenterX + (int16_t)(cos(spokeAngle) * (crankRadius - 2));
                int16_t spokeEndY = crankCenterY + (int16_t)(sin(spokeAngle) * (crankRadius - 2));
                display->drawLine(crankCenterX, crankCenterY, spokeEndX, spokeEndY, COLOR_DIM);
            }
            
            // Rotating handle (angle based on duration, counter-clockwise from bottom-left)
            float handleAngle = (135.0f + ((duration - 1) * 270.0f / 7.0f)) * PI / 180.0f;
            int16_t handleX = crankCenterX + (int16_t)(cos(handleAngle) * crankRadius);
            int16_t handleY = crankCenterY + (int16_t)(sin(handleAngle) * crankRadius);
            
            // Handle color
            uint16_t handleColor = COLOR_DIAL;
            if (i == editStep && editMode == 2) handleColor = COLOR_HIGHLIGHT;  // Highlight only when editing crank (mode 2)
            
            // Draw connecting line and handle
            display->drawLine(crankCenterX, crankCenterY, handleX, handleY, handleColor);
            display->fillCircle(handleX, handleY, 2, handleColor);
            
            // Center hub
            display->fillCircle(crankCenterX, crankCenterY, 2, COLOR_FG);
            
            // Beat count below
            char beatText[4];
            snprintf(beatText, sizeof(beatText), "%d", duration);
            display->drawText(crankCenterX - 2, crankY + 45, beatText, COLOR_FG, FONT_SMALL);
            
            // Show spinning animation if this is current step and ratcheting
            if (i == currentStep && duration > 1 && scriptSlots[slot].seqCurrentBeat > 0) {
                // Draw beat indicator dots
                uint8_t currentBeat = scriptSlots[slot].seqCurrentBeat;
                for (uint8_t b = 0; b < currentBeat && b < duration; b++) {
                    int16_t dotX = crankCenterX - (duration * 2) + (b * 4) + 2;
                    int16_t dotY = crankY + 55;
                    display->fillCircle(dotX, dotY, 1, COLOR_ACCENT);
                }
            }
            
            // Draw highlight box around entire crank when editing (mode 2)
            if (i == editStep && editMode == 2) {
                int16_t boxSize = crankRadius * 2 + 6;
                display->drawRect(crankCenterX - boxSize/2, crankY, boxSize, crankH, COLOR_HIGHLIGHT);
                display->drawRect(crankCenterX - boxSize/2 + 1, crankY + 1, boxSize - 2, crankH - 2, COLOR_HIGHLIGHT);
            }
        }
    }
    
    // Track beat count changes for next frame
    if (scriptSlots[slot].seqCurrentBeat != scriptSlots[slot].lastSeqCurrentBeat) {
        // Clear old beat indicators when beat count changes
        if (scriptSlots[slot].lastSeqCurrentBeat != 255) {
            int16_t crankCenterX = leverStartX + leverWidth / 2 + currentStep * (leverWidth + leverSpacing);
            display->fillRect(crankCenterX - 16, crankY + 52, 32, 8, COLOR_BG);
        }
        scriptSlots[slot].lastSeqCurrentBeat = scriptSlots[slot].seqCurrentBeat;
    }
    
    // Trigger indication is now via red flash on lever handle, no separate animation needed
    
    // Update tracking state
    scriptSlots[slot].lastSeqCurrentStep = currentStep;
    scriptSlots[slot].lastSeqEditStep = editStep;
    scriptSlots[slot].lastSeqEditMode = scriptSlots[slot].seqEditMode;
    for (int i = 0; i < 8; i++) {
        scriptSlots[slot].lastSeqStepValues[i] = scriptSlots[slot].seqStepValues[i];
        scriptSlots[slot].lastSeqStepDurations[i] = scriptSlots[slot].seqStepDurations[i];
        scriptSlots[slot].lastSeqGateModes[i] = scriptSlots[slot].seqGateModes[i];
    }
}
