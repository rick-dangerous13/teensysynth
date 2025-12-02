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
        scriptSlots[i].seqCurrentStep = 0;
        scriptSlots[i].seqEditStep = 0;
        scriptSlots[i].seqEditingDuration = false;
        scriptSlots[i].lastSeqCurrentStep = 255;  // 255 = uninitialized
        scriptSlots[i].lastSeqEditStep = 255;
        scriptSlots[i].lastSeqEditingDuration = false;
        for (int j = 0; j < 8; j++) {
            scriptSlots[i].seqStepValues[j] = 0;
            scriptSlots[i].seqStepDurations[j] = 1;
            scriptSlots[i].lastSeqStepValues[j] = 0;
            scriptSlots[i].lastSeqStepDurations[j] = 1;
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
        display->drawText(x + 20, y + 5, scriptSlots[slot].name, COLOR_FG, FONT_MEDIUM);
        
        // Draw running indicator (cyan dot)
        display->fillCircle(x + w - 10, y + 10, 4, COLOR_ACCENT);
        
        // Debug: Show script type
        char debugType[16];
        snprintf(debugType, sizeof(debugType), "T:%d", scriptSlots[slot].scriptType);
        display->drawText(x + 5, y + 18, debugType, COLOR_DIM, FONT_SMALL);
        
        // Render based on script type
        if (scriptSlots[slot].scriptType == 0) {
            // LFO - draw waveform visualization
            bool parametersChanged = strcmp(scriptSlots[slot].path, scriptSlots[slot].lastPath) != 0;
            
            int16_t waveX = x + 8;
            int16_t waveY = y + 25;
            int16_t waveW = w - 20;
            int16_t waveH = 60;
            int16_t waveCenterY = waveY + waveH / 2;
            
            if (parametersChanged) {
                display->fillRect(x + 6, waveY - 2, w - 12, h - 22, COLOR_BG);
                strncpy(scriptSlots[slot].lastPath, scriptSlots[slot].path, sizeof(scriptSlots[slot].lastPath) - 1);
                scriptSlots[slot].lastPath[sizeof(scriptSlots[slot].lastPath) - 1] = '\0';
            } else {
                display->fillRect(waveX, waveY, waveW, waveH, COLOR_BG);
            }
            
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
            
            // Draw parameters
            int16_t textY = waveY + waveH + 5;
            if (parametersChanged && strlen(scriptSlots[slot].path) > 0) {
                char outputCopy[128];
                strncpy(outputCopy, scriptSlots[slot].path, sizeof(outputCopy) - 1);
                outputCopy[sizeof(outputCopy) - 1] = '\0';
                
                char* line = strtok(outputCopy, "\n");
                if (line != nullptr && strcmp(line, scriptSlots[slot].name) == 0) {
                    line = strtok(nullptr, "\n");
                }
                
                int lineCount = 0;
                while (line != nullptr && textY < (y + h - 5) && lineCount < 3) {
                    display->drawText(x + 8, textY, line, COLOR_FG, FONT_SMALL);
                    textY += 10;
                    line = strtok(nullptr, "\n");
                    lineCount++;
                }
            }
        } else if (scriptSlots[slot].scriptType == 1) {
            // Sequencer - split screen: top 3/4 for sliders, bottom 1/4 for dials
            int16_t contentY = y + 25;
            int16_t contentH = h - 30;
            int16_t sliderH = (contentH * 3) / 4;
            int16_t dialH = contentH - sliderH;
            
            drawSequencerSliders(slot, x, contentY, w, sliderH);
            drawSequencerDials(slot, x, contentY + sliderH, w, dialH);
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
        // Toggle between pitch and duration
        if (!scriptSlots[slot].seqEditingDuration) {
            // Was editing pitch, now edit duration
            scriptSlots[slot].seqEditingDuration = true;
        } else {
            // Was editing duration, advance to next step's pitch
            scriptSlots[slot].seqEditingDuration = false;
            scriptSlots[slot].seqEditStep = (scriptSlots[slot].seqEditStep + 1) % 8;
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
