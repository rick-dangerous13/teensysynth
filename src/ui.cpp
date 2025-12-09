/**
 * UI System Implementation
 * 
 * Norns-style graphical user interface
 */

#include "ui.h"
#include "script_manager.h"
#include "chord_sequencer_script.h"
#include <string.h>
#include <cmath>
#include <stdio.h>
#include <math.h>
#include <ctype.h>

// Helper function to convert string to lowercase
static void toLowercase(char* dest, const char* src, size_t maxLen) {
    size_t i;
    for (i = 0; i < maxLen - 1 && src[i] != '\0'; i++) {
        dest[i] = tolower((unsigned char)src[i]);
    }
    dest[i] = '\0';
}

UI::UI() : display(nullptr), scriptManager(nullptr), clockTempo(DEFAULT_CLOCK_BPM), multitaskingMode(false), menuSelection(0), lastMenuSelection(-1), menuItemCount(0), scrollOffset(0), selectedScriptSlot(0) {
    // Initialize button strip tracking
    for (int i = 0; i < 4; i++) {
        lastButtonLabels[i][0] = '\0';
    }
    
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
            scriptSlots[i].seqToggleDirection[j] = 0;  // Start going toward up
            scriptSlots[i].lastSeqStepValues[j] = 0;
            scriptSlots[i].lastSeqStepDurations[j] = 1;
            scriptSlots[i].lastSeqGateModes[j] = 0;
        }
        // ChordSequencer initialization
        scriptSlots[i].currentChordSlot = 0;
        scriptSlots[i].chordBeatCounter = 0;
        scriptSlots[i].lastCurrentChordSlot = 255;  // 255 = uninitialized
        scriptSlots[i].lastChordBeatCounter = 255;
        scriptSlots[i].carouselActive = false;
        scriptSlots[i].carouselSelectedIdx = 0;
        scriptSlots[i].lastCarouselActive = false;
        scriptSlots[i].lastCarouselSelectedIdx = 255;
        scriptSlots[i].carouselScrollOffset = 0;
        scriptSlots[i].lastCarouselScrollOffset = 0;
        scriptSlots[i].chordCount = 0;
        scriptSlots[i].lastChordCount = 255;
        scriptSlots[i].chordListActive = false;
        scriptSlots[i].lastChordListActive = false;
        scriptSlots[i].chordListSelectedIdx = 0;
        scriptSlots[i].chordListTargetSlot = 0;
        scriptSlots[i].lastChordListSelectedIdx = 255;
        scriptSlots[i].beatCountPickerActive = false;
        scriptSlots[i].beatCountSelection = 8;
        scriptSlots[i].lastBeatCountSelection = 255;
        scriptSlots[i].lastBeatCountPickerActive = false;
        // Global parameters
        scriptSlots[i].selectedGlobalParam = 255;  // None selected initially
        scriptSlots[i].editingGlobalParam = false;
        scriptSlots[i].globalKey = 0;  // C
        scriptSlots[i].globalDegree = 0;  // Major
        scriptSlots[i].globalTheoryMode = 0;  // Functional
        scriptSlots[i].globalVoiceLeading = 0.5f;
        scriptSlots[i].globalEnergy = 0.5f;
        scriptSlots[i].globalParamNeedsRedraw = true;  // Force initial draw
        scriptSlots[i].lastSelectedGlobalParam = 255;
        scriptSlots[i].lastEditingGlobalParam = false;
        scriptSlots[i].lastGlobalKey = 255;
        scriptSlots[i].lastGlobalDegree = 255;
        scriptSlots[i].lastGlobalTheoryMode = 255;
        scriptSlots[i].lastGlobalVoiceLeading = -1.0f;
        scriptSlots[i].lastGlobalEnergy = -1.0f;
        
        // Ranked chords (Package 4)
        scriptSlots[i].rankedChordCount = 0;
        scriptSlots[i].rankedChordsValid = false;
        
        for (int j = 0; j < MAX_CHORD_SLOTS; j++) {
            scriptSlots[i].chordRoots[j] = 0;  // Default to C
            scriptSlots[i].chordTypes[j] = 0;  // Default to major
            scriptSlots[i].chordBeats[j] = 16;  // Default to 16 beats
            scriptSlots[i].lastChordRoots[j] = 255;
            scriptSlots[i].lastChordTypes[j] = 255;
            scriptSlots[i].lastChordBeats[j] = 255;
        }
    }
    
    // Initialize chord library (12 common chords: all 12 notes in major and minor)
    // We'll use: C, D, E, F, G, A, B major and minor versions
    chordLibrary[0] = {0, 0};   // C maj
    chordLibrary[1] = {0, 1};   // C min
    chordLibrary[2] = {2, 0};   // D maj
    chordLibrary[3] = {2, 1};   // D min
    chordLibrary[4] = {4, 0};   // E maj
    chordLibrary[5] = {4, 1};   // E min
    chordLibrary[6] = {5, 0};   // F maj
    chordLibrary[7] = {5, 1};   // F min
    chordLibrary[8] = {7, 0};   // G maj
    chordLibrary[9] = {7, 1};   // G min
    chordLibrary[10] = {9, 0};  // A maj
    chordLibrary[11] = {9, 1};  // A min
    
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
        // Main menu has no BACK button (top level)
        drawButtonStrip("", "select", "", "");
        
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
        
        // Reset drawing state for all active scripts to force full redraw (only on initial display)
        for (int i = 0; i < MAX_SCRIPTS; i++) {
            if (scriptSlots[i].active) {
                // Reset chord sequencer state
                scriptSlots[i].lastCurrentChordSlot = 255;
                scriptSlots[i].lastChordBeatCounter = 255;
                // Reset poliquencer state
                scriptSlots[i].lastSeqCurrentStep = 255;
                scriptSlots[i].lastSeqCurrentBeat = 255;
                // Reset LFO state
                scriptSlots[i].lastLfoWaveType = 255;
            }
        }
    }
    
    // Always redraw script slots - only slot 0 for now
    menuSelection = 0;  // Force selection to slot 0
    selectedScriptSlot = 0;
    drawScriptSlot(0, true);  // Always selected since it's the only available slot
}

void UI::showScriptLibraryScreen(ScriptManager* scriptMgr) {
    if (!display) return;
    
    // Note: menuItemCount should be set from main.cpp using script manager
    // Default to 5 if not set
    if (menuItemCount == 0) menuItemCount = 5;
    
    // Only do full redraw if this is initial display
    if (lastMenuSelection == -1) {
        display->clear();
        
        // Title at top-left like Poliquencer
        display->drawText(20, 5, "Select Script", COLOR_FG, FONT_SMALL);
        
        drawFooter("OK: load", "BACK: cancel");
        
        // Get actual script library items from script manager
        int16_t startY = 50;
        for (int i = 0; i < menuItemCount; i++) {
            const char* name = "Unknown";
            if (scriptMgr) {
                const ScriptLibraryEntry* entry = scriptMgr->getScriptLibraryEntry(i);
                if (entry) name = entry->name;
            }
            drawMenuItem(startY + i * MENU_ITEM_H, name, i == menuSelection);
        }
    } else if (lastMenuSelection != menuSelection) {
        // Only redraw the changed menu items
        int16_t startY = 50;
        
        if (lastMenuSelection >= 0 && lastMenuSelection < menuItemCount) {
            const char* name = "Unknown";
            if (scriptMgr) {
                const ScriptLibraryEntry* entry = scriptMgr->getScriptLibraryEntry(lastMenuSelection);
                if (entry) name = entry->name;
            }
            drawMenuItem(startY + lastMenuSelection * MENU_ITEM_H, name, false);
        }
        
        if (menuSelection >= 0 && menuSelection < menuItemCount) {
            const char* name = "Unknown";
            if (scriptMgr) {
                const ScriptLibraryEntry* entry = scriptMgr->getScriptLibraryEntry(menuSelection);
                if (entry) name = entry->name;
            }
            drawMenuItem(startY + menuSelection * MENU_ITEM_H, name, true);
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
        "Input Test",
        "Audio Output",
        "MIDI Channel",
        "Display Brightness",
        "Script Auto-load"
    };
    
    menuItemCount = 7;
    
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
        // Full screen mode - reserve bottom 15px for button strip
        x = 0;
        y = 0;
        w = SCREEN_WIDTH;
        h = 225;  // Leave room for button strip at y=225
    }
    
    // No selection border - clean interface
    
    // Draw slot number
    char slotNum[4];
    snprintf(slotNum, sizeof(slotNum), "%d", slot + 1);
    display->drawText(x + 5, y + 5, slotNum, COLOR_DIM, FONT_SMALL);
    
    // Draw script name or "empty"
    if (scriptSlots[slot].active) {
        if (scriptSlots[slot].scriptType == 2) {
            // Poliquencer title at top
            display->drawText(x + 20, y + 5, "POLIQUENCER", COLOR_FG, FONT_SMALL);
        } else if (scriptSlots[slot].scriptType == 5) {
            // Symphony chord sequencer title, same size/position as poliquencer
            display->drawText(x + 20, y + 5, "SYMPHONY CHROD SEQUENCER", COLOR_FG, FONT_SMALL);
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
            // LFO - draw waveform visualization + editable parameters (condensed for button strip)
            int16_t contentY = y + 30;
            int16_t contentH = h - 30;  // Adjusted for button strip
            
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
            // Sequencer - split screen: top for sliders, bottom for dials (condensed for button strip)
            int16_t contentY = y + 25;
            int16_t contentH = h - 25;  // Adjusted for button strip
            int16_t sliderH = (contentH * 3) / 4;
            int16_t dialH = contentH - sliderH;
            
            drawSequencerSliders(slot, x, contentY, w, sliderH);
            drawSequencerDials(slot, x, contentY + sliderH, w, dialH);
        } else if (scriptSlots[slot].scriptType == 2) {
            // Poliquencer - full poliquencer aesthetic (condensed for button strip)
            int16_t contentY = y + 20;
            int16_t contentH = h - 20;  // Adjusted for button strip
            drawPoliquencerSequencer(slot, x, contentY, w, contentH);
        } else if (scriptSlots[slot].scriptType == 5) {
            // ChordSequencer - Oxi One style chord progression display
            int16_t contentY = y + 25;
            int16_t contentH = h - 25;  // Adjusted for button strip
            drawChordSequencer(slot, x, contentY, w, contentH);
        } else if (scriptSlots[slot].scriptType == 3) {
            // Touch Calibration Test - show targets
            display->fillRect(x + 6, y + 25, w - 12, h - 30, COLOR_BG);
            
            // Draw title
            display->drawText(x + 8, y + 40, "Touch Calibration", COLOR_ACCENT, FONT_MEDIUM);
            display->drawText(x + 8, y + 65, "Touch each corner", COLOR_FG, FONT_SMALL);
            display->drawText(x + 8, y + 80, "and center. Watch", COLOR_FG, FONT_SMALL);
            display->drawText(x + 8, y + 95, "Serial for coords", COLOR_FG, FONT_SMALL);
            
            // Draw calibration targets (crosshairs)
            // Top-left
            display->drawLine(5, 5, 15, 5, COLOR_ACCENT);
            display->drawLine(10, 0, 10, 10, COLOR_ACCENT);
            // Top-right
            display->drawLine(SCREEN_WIDTH - 15, 5, SCREEN_WIDTH - 5, 5, COLOR_ACCENT);
            display->drawLine(SCREEN_WIDTH - 10, 0, SCREEN_WIDTH - 10, 10, COLOR_ACCENT);
            // Bottom-left
            display->drawLine(5, SCREEN_HEIGHT - 5, 15, SCREEN_HEIGHT - 5, COLOR_ACCENT);
            display->drawLine(10, SCREEN_HEIGHT - 10, 10, SCREEN_HEIGHT, COLOR_ACCENT);
            // Bottom-right
            display->drawLine(SCREEN_WIDTH - 15, SCREEN_HEIGHT - 5, SCREEN_WIDTH - 5, SCREEN_HEIGHT - 5, COLOR_ACCENT);
            display->drawLine(SCREEN_WIDTH - 10, SCREEN_HEIGHT - 10, SCREEN_WIDTH - 10, SCREEN_HEIGHT, COLOR_ACCENT);
            // Center
            display->drawLine(SCREEN_WIDTH/2 - 10, SCREEN_HEIGHT/2, SCREEN_WIDTH/2 + 10, SCREEN_HEIGHT/2, COLOR_HIGHLIGHT);
            display->drawLine(SCREEN_WIDTH/2, SCREEN_HEIGHT/2 - 10, SCREEN_WIDTH/2, SCREEN_HEIGHT/2 + 10, COLOR_HIGHLIGHT);
        } else {
            // Unknown script type - show error
            display->fillRect(x + 6, y + 25, w - 12, h - 30, COLOR_BG);
            display->drawText(x + 8, y + 40, "Unknown type", COLOR_DIM, FONT_SMALL);
        }
        
        // Draw button strip for running scripts
        drawButtonStrip("BACK", "edit", "", "");
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
    
    // Convert old footer API to new button strip
    // Button 1 (leftmost) = Back (if rightLabel contains "back" or "BACK")
    // Button 2 = Select (if leftLabel is not empty)
    // Buttons 3 & 4 = empty for now
    
    const char* btn1 = "";
    const char* btn2 = "";
    const char* btn3 = "";
    const char* btn4 = "";
    
    // Check if back button should be shown
    if (rightLabel && strlen(rightLabel) > 0) {
        // Look for "back" in the label
        const char* backPos = strstr(rightLabel, "back");
        const char* backPosUpper = strstr(rightLabel, "BACK");
        if (backPos || backPosUpper) {
            btn1 = "BACK";
        }
    }
    
    // Check if select/OK button should be shown
    if (leftLabel && strlen(leftLabel) > 0) {
        // Parse the label to extract action
        const char* colonPos = strchr(leftLabel, ':');
        if (colonPos) {
            // Extract text after "OK: " or "OK:"
            const char* actionStart = colonPos + 1;
            while (*actionStart == ' ') actionStart++;  // Skip spaces
            btn2 = actionStart;
        } else {
            btn2 = "OK";
        }
    }
    
    drawButtonStrip(btn1, btn2, btn3, btn4);
}

void UI::drawGlobalParameterBoxes(uint8_t slot, int16_t x, int16_t y, int16_t w, int16_t h) {
    if (!display || slot >= MAX_SCRIPTS) return;
    
    // Note names and theory mode names for display
    static const char* noteNames[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
    static const char* degreeNames[] = {"maj", "min", "dor", "phry", "lyd", "mix", "loc"};
    static const char* theoryModeNames[] = {"func", "diat", "modal", "chrom", "all"};
    
    // Layout: 5 boxes in a row below the chord progression
    // Position them below the dots area
    int16_t boxY = y + h - 35;  // Near bottom of slot
    int16_t boxH = 28;
    int16_t boxW = 58;  // Narrower to fit 5 boxes
    int16_t boxGap = 4;
    int16_t boxStartX = x + 10;
    
    // Box data: label, value string
    const char* labels[] = {"key", "degree", "theory", "compact", "energy"};
    char values[5][16];
    
    // Check if this needs a full redraw (from slot state, not static)
    bool isFirstDraw = scriptSlots[slot].globalParamNeedsRedraw;
    
    // Format values
    snprintf(values[0], sizeof(values[0]), "%s", noteNames[scriptSlots[slot].globalKey]);
    snprintf(values[1], sizeof(values[1]), "%s", degreeNames[scriptSlots[slot].globalDegree]);
    snprintf(values[2], sizeof(values[2]), "%s", theoryModeNames[scriptSlots[slot].globalTheoryMode]);
    snprintf(values[3], sizeof(values[3]), "%.2f", scriptSlots[slot].globalVoiceLeading);
    snprintf(values[4], sizeof(values[4]), "%.2f", scriptSlots[slot].globalEnergy);
    
    for (int i = 0; i < 5; i++) {
        int16_t boxX = boxStartX + i * (boxW + boxGap);
        
        // Determine current state for this specific box
        bool isSelected = (scriptSlots[slot].selectedGlobalParam == i);
        bool isEditing = (isSelected && scriptSlots[slot].editingGlobalParam);
        bool wasSelected = (scriptSlots[slot].lastSelectedGlobalParam == i);
        bool wasEditing = (wasSelected && scriptSlots[slot].lastEditingGlobalParam);
        
        // Only redraw if: first draw, selection state changed, OR this box is selected and a value changed
        bool selectionStateChanged = (isSelected != wasSelected) || (isEditing != wasEditing);
        
        // Check if values actually changed
        bool anyValueChanged = false;
        if (scriptSlots[slot].globalKey != scriptSlots[slot].lastGlobalKey) anyValueChanged = true;
        if (scriptSlots[slot].globalDegree != scriptSlots[slot].lastGlobalDegree) anyValueChanged = true;
        if (scriptSlots[slot].globalTheoryMode != scriptSlots[slot].lastGlobalTheoryMode) anyValueChanged = true;
        if (fabs(scriptSlots[slot].globalVoiceLeading - scriptSlots[slot].lastGlobalVoiceLeading) > 0.001f) anyValueChanged = true;
        if (fabs(scriptSlots[slot].globalEnergy - scriptSlots[slot].lastGlobalEnergy) > 0.001f) anyValueChanged = true;
        
        // Redraw if: first draw, selection changed, OR if value changed and box is selected
        bool needsRedraw = isFirstDraw || selectionStateChanged || (anyValueChanged && isSelected);
        
        if (!needsRedraw) continue;
        
        uint16_t boxColor;
        uint16_t textColor;
        
        if (isEditing) {
            boxColor = COLOR_HIGHLIGHT;  // Yellow when editing
            textColor = COLOR_BG;
        } else if (isSelected) {
            boxColor = COLOR_ACCENT;  // Cyan when selected
            textColor = COLOR_BG;
        } else {
            boxColor = 0x1082;  // Dark gray when not selected
            textColor = COLOR_FG;
        }
        
        // Draw box
        display->fillRoundRect(boxX, boxY, boxW, boxH, 4, boxColor);
        
        // Draw label (top)
        int16_t labelY = boxY + 6;
        display->drawText(boxX + 3, labelY, labels[i], textColor, FONT_SMALL);
        
        // Draw value (bottom, centered)
        int16_t valueY = boxY + 18;
        int16_t bx, by; uint16_t bw, bh;
        display->getTextBounds(values[i], 0, 0, &bx, &by, &bw, &bh, FONT_SMALL);
        int16_t valueX = boxX + (boxW - bw) / 2 - bx;
        display->drawText(valueX, valueY, values[i], textColor, FONT_SMALL);
    }
    
    // Update last values for change detection
    scriptSlots[slot].lastSelectedGlobalParam = scriptSlots[slot].selectedGlobalParam;
    scriptSlots[slot].lastEditingGlobalParam = scriptSlots[slot].editingGlobalParam;
    scriptSlots[slot].lastGlobalKey = scriptSlots[slot].globalKey;
    scriptSlots[slot].lastGlobalDegree = scriptSlots[slot].globalDegree;
    scriptSlots[slot].lastGlobalTheoryMode = scriptSlots[slot].globalTheoryMode;
    scriptSlots[slot].lastGlobalVoiceLeading = scriptSlots[slot].globalVoiceLeading;
    scriptSlots[slot].lastGlobalEnergy = scriptSlots[slot].globalEnergy;
    
    // Clear the redraw flag after drawing
    scriptSlots[slot].globalParamNeedsRedraw = false;
}

void UI::drawChordSequencer(uint8_t slot, int16_t x, int16_t y, int16_t w, int16_t h) {
    if (!display || slot >= MAX_SCRIPTS) return;

    // If overlay just closed, clear its area before redrawing base UI to avoid blanking afterward
    if (!scriptSlots[slot].chordListActive && scriptSlots[slot].lastChordListActive) {
        int16_t overlayX = x + 4;
        int16_t overlayW = w - 8;
        int16_t overlayY = y - 10;
        int16_t overlayH = h + 20;
        if (overlayY < 0) overlayY = 0;
        if (overlayY + overlayH > SCREEN_HEIGHT) overlayH = SCREEN_HEIGHT - overlayY;
        display->fillRect(overlayX, overlayY, overlayW, overlayH, COLOR_BG);
        scriptSlots[slot].lastChordListActive = false;
    }

    bool overlayBlocksBase = scriptSlots[slot].chordListActive || scriptSlots[slot].beatCountPickerActive || scriptSlots[slot].carouselActive;
    bool lastOverlayBlocks = scriptSlots[slot].lastChordListActive || scriptSlots[slot].lastBeatCountPickerActive || scriptSlots[slot].lastCarouselActive;
    bool overlayJustClosed = (!overlayBlocksBase && lastOverlayBlocks);

    // Detect chord count change to force full redraw (ensures + box and new chord show up immediately)
    bool chordCountChanged = (scriptSlots[slot].chordCount != scriptSlots[slot].lastChordCount);

    bool firstDraw = overlayJustClosed || chordCountChanged || (scriptSlots[slot].lastCurrentChordSlot == 255);
    
    // Force global parameter redraw when overlay just closed or on first draw
    if (overlayJustClosed || firstDraw) {
        scriptSlots[slot].globalParamNeedsRedraw = true;
    }

    // When an overlay is active, keep base visuals off and force full redraw when it closes
    if (overlayBlocksBase) {
        scriptSlots[slot].lastCurrentChordSlot = 255;
        scriptSlots[slot].lastChordBeatCounter = 255;
        scriptSlots[slot].lastSelectedChordSlot = 255;
        if (!lastOverlayBlocks) {
            display->fillRect(x + 6, y, w - 12, h, COLOR_BG);
        }
    }
    
    // Note names for display
    const char* noteNames[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
    const char* typeNames[] = {"maj", "min"};

    // Clamp selection to existing chords (+ optional plus box)
    uint8_t chordCount = scriptSlots[slot].chordCount;
    if (chordCount > MAX_CHORD_SLOTS) chordCount = MAX_CHORD_SLOTS;
    bool hasChords = (chordCount > 0);
    bool hasPlusBox = (chordCount < MAX_CHORD_SLOTS);

    // plus box is always addressable at index chordCount when capacity remains; if no chords, plus box sits at 0
    uint8_t maxSelectable = hasPlusBox ? (chordCount + 1) : chordCount;
    if (maxSelectable == 0) maxSelectable = 1;  // Ensure plus box selectable when empty
    if (scriptSlots[slot].selectedChordSlot >= maxSelectable) {
        scriptSlots[slot].selectedChordSlot = maxSelectable - 1;
    }
    
    // Calculate total beats
    uint16_t totalBeats = 0;
    for (int i = 0; i < chordCount; i++) {
        totalBeats += scriptSlots[slot].chordBeats[i];
    }
    if (totalBeats == 0) totalBeats = 1;  // Avoid division by zero
    
    // Layout: Fixed progression width (260px), then + box (20px)
    int16_t progressionW = 260;  // Fixed width for step progression
    int16_t plusBoxW = 20;
    int16_t gapBetween = 6;
    int16_t startX = x + 10;  // Left padding
    int16_t plusBoxX = startX + progressionW + gapBetween;
    
    // Vertical positioning: center the progression in available height
    int16_t chartH = 40;  // Height of chord boxes
    int16_t chartY = y + (h - chartH) / 2;
    
    // Step calculations
    float pixelsPerStep = (float)progressionW / (float)totalBeats;
    
    // Helper: compute absolute beat index for current chord/beat
    auto getAbsoluteBeat = [&](void) -> uint16_t {
        uint16_t offset = 0;
        for (uint8_t i = 0; i < scriptSlots[slot].currentChordSlot && i < chordCount; i++) {
            offset += scriptSlots[slot].chordBeats[i];
        }
        return offset + scriptSlots[slot].chordBeatCounter;
    };
    
    if (!overlayBlocksBase) {
        if (firstDraw) {
            // Full redraw: clear, draw all chord boxes, draw dots, draw plus box
            display->fillRect(x + 6, y, w - 12, h, COLOR_BG);
            
            // Draw chord boxes with rounded edges
            int16_t beatX = startX;
            for (int i = 0; i < chordCount; i++) {
                // Calculate box position and width based on beat count
                float boxW = scriptSlots[slot].chordBeats[i] * pixelsPerStep;
                int16_t boxWidth = (int16_t)roundf(boxW);
                if (i == chordCount - 1) {
                    // Last box extends to fill remaining space
                    boxWidth = startX + progressionW - (int16_t)beatX;
                }
                
                bool isSelected = (i == scriptSlots[slot].selectedChordSlot);
                uint16_t boxColor = isSelected ? COLOR_ACCENT : 0x5D9F;
                
                // Draw rounded rectangle for chord
                display->fillRoundRect(beatX, chartY, boxWidth, chartH, 6, boxColor);
                
                // Draw chord label (root + type) centered in box
                char label[8];
                snprintf(label, sizeof(label), "%s%s", noteNames[scriptSlots[slot].chordRoots[i]], 
                         typeNames[scriptSlots[slot].chordTypes[i]]);
                
                int16_t bx, by; uint16_t bw, bh;
                display->getTextBounds(label, 0, 0, &bx, &by, &bw, &bh, FONT_SMALL);
                int16_t labelX = beatX + (boxWidth - bw) / 2 - bx;
                int16_t labelY = chartY + (chartH - bh) / 2 - by;
                display->drawText(labelX, labelY, label, COLOR_BG, FONT_SMALL);
                
                beatX += boxWidth;
            }
            
            // Draw step indicator dots below chord boxes
            int16_t dotY = chartY + chartH + 8;
            for (int step = 0; step < totalBeats; step++) {
                int16_t dotX = startX + (int16_t)(step * pixelsPerStep + pixelsPerStep / 2);
                
                // Highlight current step
                uint16_t dotColor = (step == getAbsoluteBeat()) ? COLOR_ACCENT : COLOR_DIM;
                display->fillCircle(dotX, dotY, 2, dotColor);
            }
            
            // Draw plus box for adding chords
            if (hasPlusBox) {
                uint16_t plusColor = (scriptSlots[slot].selectedChordSlot == chordCount) ? COLOR_ACCENT : 0x1082;
                int16_t plusBoxY = chartY + (chartH / 2) - 10;
                int16_t plusBoxH = 20;
                display->fillRoundRect(plusBoxX, plusBoxY, plusBoxW, plusBoxH, 4, plusColor);
                int16_t bx, by; uint16_t bw, bh;
                display->getTextBounds("+", 0, 0, &bx, &by, &bw, &bh, FONT_MEDIUM);
                int16_t textX = plusBoxX + (plusBoxW - bw) / 2 - bx;
                int16_t textY = plusBoxY + (plusBoxH - bh) / 2 - by;
                display->drawText(textX, textY, "+", COLOR_BG, FONT_MEDIUM);
            }
            
            // Mark as initialized
            scriptSlots[slot].lastCurrentChordSlot = scriptSlots[slot].currentChordSlot;
            scriptSlots[slot].lastChordBeatCounter = scriptSlots[slot].chordBeatCounter;
            scriptSlots[slot].lastSelectedChordSlot = scriptSlots[slot].selectedChordSlot;
            for (int i = 0; i < chordCount; i++) {
                scriptSlots[slot].lastChordRoots[i] = scriptSlots[slot].chordRoots[i];
                scriptSlots[slot].lastChordTypes[i] = scriptSlots[slot].chordTypes[i];
                scriptSlots[slot].lastChordBeats[i] = scriptSlots[slot].chordBeats[i];
            }
        } else {
            // Check if beats changed - need full redraw of chord boxes
            bool beatsChanged = false;
            for (int i = 0; i < chordCount; i++) {
                if (scriptSlots[slot].chordBeats[i] != scriptSlots[slot].lastChordBeats[i]) {
                    beatsChanged = true;
                    break;
                }
            }
            
            if (beatsChanged || chordCountChanged) {
                // Clear and redraw chord boxes and dots
                display->fillRect(startX, chartY - 5, progressionW, chartH + 20, COLOR_BG);
                
                int16_t beatX = startX;
                for (int i = 0; i < chordCount; i++) {
                    float boxW = scriptSlots[slot].chordBeats[i] * pixelsPerStep;
                    int16_t boxWidth = (int16_t)roundf(boxW);
                    if (i == chordCount - 1) {
                        boxWidth = startX + progressionW - (int16_t)beatX;
                    }
                    
                    bool isSelected = (i == scriptSlots[slot].selectedChordSlot);
                    uint16_t boxColor = isSelected ? COLOR_ACCENT : 0x5D9F;
                    
                    display->fillRoundRect(beatX, chartY, boxWidth, chartH, 6, boxColor);
                    
                    char label[8];
                    snprintf(label, sizeof(label), "%s%s", noteNames[scriptSlots[slot].chordRoots[i]], 
                             typeNames[scriptSlots[slot].chordTypes[i]]);
                    
                    int16_t bx, by; uint16_t bw, bh;
                    display->getTextBounds(label, 0, 0, &bx, &by, &bw, &bh, FONT_SMALL);
                    int16_t labelX = beatX + (boxWidth - bw) / 2 - bx;
                    int16_t labelY = chartY + (chartH - bh) / 2 - by;
                    display->drawText(labelX, labelY, label, COLOR_BG, FONT_SMALL);
                    
                    beatX += boxWidth;
                }
                
                // Redraw dots after beat change
                int16_t dotY = chartY + chartH + 8;
                for (int step = 0; step < totalBeats; step++) {
                    int16_t dotX = startX + (int16_t)(step * pixelsPerStep + pixelsPerStep / 2);
                    uint16_t dotColor = (step == getAbsoluteBeat()) ? COLOR_ACCENT : COLOR_DIM;
                    display->fillCircle(dotX, dotY, 2, dotColor);
                }
                
                // Update cached beat counts
                for (int i = 0; i < chordCount; i++) {
                    scriptSlots[slot].lastChordBeats[i] = scriptSlots[slot].chordBeats[i];
                }
                
                // Redraw plus box after beat change
                if (hasPlusBox) {
                    uint16_t plusColor = (scriptSlots[slot].selectedChordSlot == chordCount) ? COLOR_ACCENT : 0x1082;
                    int16_t plusBoxY = chartY + (chartH / 2) - 10;
                    int16_t plusBoxH = 20;
                    display->fillRoundRect(plusBoxX, plusBoxY, plusBoxW, plusBoxH, 4, plusColor);
                    int16_t bx, by; uint16_t bw, bh;
                    display->getTextBounds("+", 0, 0, &bx, &by, &bw, &bh, FONT_MEDIUM);
                    int16_t textX = plusBoxX + (plusBoxW - bw) / 2 - bx;
                    int16_t textY = plusBoxY + (plusBoxH - bh) / 2 - by;
                    display->drawText(textX, textY, "+", COLOR_BG, FONT_MEDIUM);
                }

                // Redraw plus box whenever chord count changes (capacity/position may shift)
                if (hasPlusBox) {
                    uint16_t plusColor = (scriptSlots[slot].selectedChordSlot == chordCount) ? COLOR_ACCENT : 0x1082;
                    int16_t plusBoxY = chartY + (chartH / 2) - 10;
                    int16_t plusBoxH = 20;
                    display->fillRoundRect(plusBoxX, plusBoxY, plusBoxW, plusBoxH, 4, plusColor);
                    int16_t bx, by; uint16_t bw, bh;
                    display->getTextBounds("+", 0, 0, &bx, &by, &bw, &bh, FONT_MEDIUM);
                    int16_t textX = plusBoxX + (plusBoxW - bw) / 2 - bx;
                    int16_t textY = plusBoxY + (plusBoxH - bh) / 2 - by;
                    display->drawText(textX, textY, "+", COLOR_BG, FONT_MEDIUM);
                }
            }
            
            // Check if selection or playback position changed
            bool selectionChanged = (scriptSlots[slot].selectedChordSlot != scriptSlots[slot].lastSelectedChordSlot);
            bool playbackChanged = (scriptSlots[slot].currentChordSlot != scriptSlots[slot].lastCurrentChordSlot ||
                                   scriptSlots[slot].chordBeatCounter != scriptSlots[slot].lastChordBeatCounter);
            
            // Only redraw when something actually changed
            if (!beatsChanged && selectionChanged) {
                // Selective redraw: only redraw affected chord boxes when selection changes
                int16_t beatX = startX;
                for (int i = 0; i < chordCount; i++) {
                    float boxW = scriptSlots[slot].chordBeats[i] * pixelsPerStep;
                    int16_t boxWidth = (int16_t)roundf(boxW);
                    if (i == chordCount - 1) {
                        boxWidth = startX + progressionW - (int16_t)beatX;
                    }
                    
                    // Only redraw if this box changed selection state
                    bool wasSelected = (i == scriptSlots[slot].lastSelectedChordSlot);
                    bool isSelected = (i == scriptSlots[slot].selectedChordSlot);
                    
                    if (wasSelected != isSelected) {
                        uint16_t boxColor = isSelected ? COLOR_ACCENT : 0x5D9F;
                        display->fillRoundRect(beatX, chartY, boxWidth, chartH, 6, boxColor);
                        
                        char label[8];
                        snprintf(label, sizeof(label), "%s%s", noteNames[scriptSlots[slot].chordRoots[i]], 
                                 typeNames[scriptSlots[slot].chordTypes[i]]);
                        
                        int16_t bx, by; uint16_t bw, bh;
                        display->getTextBounds(label, 0, 0, &bx, &by, &bw, &bh, FONT_SMALL);
                        int16_t labelX = beatX + (boxWidth - bw) / 2 - bx;
                        int16_t labelY = chartY + (chartH - bh) / 2 - by;
                        display->drawText(labelX, labelY, label, COLOR_BG, FONT_SMALL);
                    }
                    
                    beatX += boxWidth;
                }
                
                // Redraw plus box if selection changed to/from it
                if (hasPlusBox) {
                    bool wasPlusSelected = (scriptSlots[slot].lastSelectedChordSlot == chordCount);
                    bool isPlusSelected = (scriptSlots[slot].selectedChordSlot == chordCount);
                    
                    if (wasPlusSelected != isPlusSelected) {
                        uint16_t plusColor = isPlusSelected ? COLOR_ACCENT : 0x1082;
                        int16_t plusBoxY = chartY + (chartH / 2) - 10;
                        int16_t plusBoxH = 20;
                        display->fillRoundRect(plusBoxX, plusBoxY, plusBoxW, plusBoxH, 4, plusColor);
                        int16_t bx, by; uint16_t bw, bh;
                        display->getTextBounds("+", 0, 0, &bx, &by, &bw, &bh, FONT_MEDIUM);
                        int16_t textX = plusBoxX + (plusBoxW - bw) / 2 - bx;
                        int16_t textY = plusBoxY + (plusBoxH - bh) / 2 - by;
                        display->drawText(textX, textY, "+", COLOR_BG, FONT_MEDIUM);
                    }
                }
            }
            
            if (!beatsChanged && playbackChanged) {
                // Only redraw playback indicator dots when position changes
                int16_t dotY = chartY + chartH + 8;
                
                // Clear old position dot
                uint16_t lastBeat = 0;
                for (uint8_t i = 0; i < scriptSlots[slot].lastCurrentChordSlot && i < chordCount; i++) {
                    lastBeat += scriptSlots[slot].chordBeats[i];
                }
                lastBeat += scriptSlots[slot].lastChordBeatCounter;
                if (lastBeat < totalBeats) {
                    int16_t lastDotX = startX + (int16_t)(lastBeat * pixelsPerStep + pixelsPerStep / 2);
                    display->fillCircle(lastDotX, dotY, 2, COLOR_DIM);
                }
                
                // Draw new position dot
                uint16_t currentBeat = getAbsoluteBeat();
                if (currentBeat < totalBeats) {
                    int16_t dotX = startX + (int16_t)(currentBeat * pixelsPerStep + pixelsPerStep / 2);
                    display->fillCircle(dotX, dotY, 2, COLOR_ACCENT);
                }
            }

            // Always redraw plus box every frame to keep it visible regardless of navigation direction
            if (hasPlusBox) {
                uint16_t plusColor = (scriptSlots[slot].selectedChordSlot == chordCount) ? COLOR_ACCENT : 0x1082;
                int16_t plusBoxY = chartY + (chartH / 2) - 10;
                int16_t plusBoxH = 20;
                display->fillRoundRect(plusBoxX, plusBoxY, plusBoxW, plusBoxH, 4, plusColor);
                int16_t bx, by; uint16_t bw, bh;
                display->getTextBounds("+", 0, 0, &bx, &by, &bw, &bh, FONT_MEDIUM);
                int16_t textX = plusBoxX + (plusBoxW - bw) / 2 - bx;
                int16_t textY = plusBoxY + (plusBoxH - bh) / 2 - by;
                display->drawText(textX, textY, "+", COLOR_BG, FONT_MEDIUM);
            } else {
                // If no plus box (full), clear its area so stale pixels don't linger
                display->fillRect(plusBoxX, chartY - 2, plusBoxW + gapBetween, chartH + 24, COLOR_BG);
            }
            
            // Update selection and playback state tracking
            scriptSlots[slot].lastSelectedChordSlot = scriptSlots[slot].selectedChordSlot;
            scriptSlots[slot].lastCurrentChordSlot = scriptSlots[slot].currentChordSlot;
            scriptSlots[slot].lastChordBeatCounter = scriptSlots[slot].chordBeatCounter;
            scriptSlots[slot].lastChordListActive = false;
            scriptSlots[slot].lastBeatCountPickerActive = false;
            scriptSlots[slot].lastCarouselActive = false;
        }
        
        // Draw global parameter boxes below chord progression (always visible)
        if (!overlayBlocksBase) {
            drawGlobalParameterBoxes(slot, x, y, w, h);
        }
    }
    
    // Draw chord list overlay if active (on top of chord sequencer)
    if (!scriptSlots[slot].beatCountPickerActive) {
        drawChordListOverlay(slot, x, y, w, h);
    }

    // Draw beat count picker overlay if active (replaces everything)
    drawBeatCountPickerOverlay(slot, x, y, w, h);

    // Track chord count for next frame
    scriptSlots[slot].lastChordCount = chordCount;
}

void UI::drawButtonStrip(const char* btn1, const char* btn2, const char* btn3, const char* btn4) {
    if (!display) return;
    
    // Button strip layout:
    // Slim strip at y=225, height 15px (no separator line or dividers)
    // 4 equal boxes across width, each 80px wide (320/4)
    // Text centered in each box, always lowercase
    // Design: Minimal, clean, no borders - just text labels
    
    const int16_t stripY = 225;
    const int16_t stripH = 15;
    const int16_t boxW = SCREEN_WIDTH / 4;  // 80px each
    
    // Button labels array
    const char* labels[4] = {btn1, btn2, btn3, btn4};
    
    // Draw each button label
    for (int i = 0; i < 4; i++) {
        int16_t boxX = i * boxW;
        int16_t textY = stripY + 4;  // Center vertically in slim strip
        
        // Convert label to lowercase for comparison and display
        char lowerLabel[16];
        toLowercase(lowerLabel, labels[i], sizeof(lowerLabel));
        
        // Clear box area if label changed
        if (strcmp(lowerLabel, lastButtonLabels[i]) != 0) {
            display->fillRect(boxX, stripY, boxW, stripH, COLOR_BG);
            
            // Draw label if not empty
            if (lowerLabel[0] != '\0') {
                // Center text horizontally in box
                int16_t x1, y1;
                uint16_t w, h;
                display->getTextBounds(lowerLabel, 0, 0, &x1, &y1, &w, &h, FONT_SMALL);
                int16_t textX = boxX + (boxW - w) / 2;
                display->drawText(textX, textY, lowerLabel, COLOR_DIM, FONT_SMALL);
            }
            
            // Update tracking
            strncpy(lastButtonLabels[i], lowerLabel, sizeof(lastButtonLabels[i]) - 1);
            lastButtonLabels[i][sizeof(lastButtonLabels[i]) - 1] = '\0';
        }
    }
}

void UI::updateButtonStrip() {
    // For future use: allow dynamic button strip updates without full redraw
    // Currently handled by drawButtonStrip's change detection
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

void UI::setSequencerEditStep(uint8_t slot, uint8_t step) {
    if (slot < MAX_SCRIPTS && step < 8) {
        scriptSlots[slot].seqEditStep = step;
    }
}

void UI::setSequencerEditMode(uint8_t slot, uint8_t mode) {
    if (slot < MAX_SCRIPTS && mode < 3) {
        scriptSlots[slot].seqEditMode = mode;
        scriptSlots[slot].seqEditingDuration = (mode == 2);
    }
}

void UI::advanceSequencerEditStep(uint8_t slot) {
    if (slot < MAX_SCRIPTS) {
        // Check if this is a poliquencer sequencer (has gate modes)
        bool isPoliquencer = (scriptSlots[slot].scriptType == 2);
        
        if (isPoliquencer) {
            // Poliquencer: cycle through lever → switch → crank → next step's lever
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

void UI::updatePoliquencerSequencer(uint8_t slot, uint8_t currentStep, uint8_t currentBeat, int8_t stepValues[8], uint8_t stepDurations[8], uint8_t gateModes[8], uint8_t direction, bool steamTrigger) {
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

void UI::updateChordSequencer(uint8_t slot, uint8_t chordRoots[MAX_CHORD_SLOTS], uint8_t chordTypes[MAX_CHORD_SLOTS], uint8_t chordBeats[MAX_CHORD_SLOTS], uint8_t currentChordSlot, uint8_t beatCounter, uint8_t chordCount) {
    if (slot >= MAX_SCRIPTS) return;

    uint8_t limitedCount = chordCount;
    if (limitedCount > MAX_CHORD_SLOTS) limitedCount = MAX_CHORD_SLOTS;

    for (int i = 0; i < MAX_CHORD_SLOTS; i++) {
        scriptSlots[slot].chordRoots[i] = chordRoots[i];
        scriptSlots[slot].chordTypes[i] = chordTypes[i];
        scriptSlots[slot].chordBeats[i] = chordBeats[i];
    }
    scriptSlots[slot].currentChordSlot = currentChordSlot;
    scriptSlots[slot].chordBeatCounter = beatCounter;
    scriptSlots[slot].chordCount = limitedCount;
    
    // Initialize selectedChordSlot if not already done
    if (scriptSlots[slot].selectedChordSlot > limitedCount) {
        scriptSlots[slot].selectedChordSlot = 0;  // Reset to first chord
        scriptSlots[slot].lastSelectedChordSlot = 255;  // Force first draw
    }
}

void UI::updateChordSequencerGlobals(uint8_t slot, const GlobalParameters& globals) {
    if (slot >= MAX_SCRIPTS) return;
    
    // Only sync if not currently editing (don't overwrite user's in-progress edits)
    if (!scriptSlots[slot].editingGlobalParam) {
        syncGlobalsFromScript(slot, globals);
    }
}

void UI::drawPoliquencerSequencer(uint8_t slot, int16_t x, int16_t y, int16_t w, int16_t h) {
    if (!display || slot >= MAX_SCRIPTS) return;
    
    bool firstDraw = (scriptSlots[slot].lastSeqCurrentStep == 255);
    
    // Layout (condensed for button strip):
    // - Top area: Levers (pitch control)
    // - Middle: Toggle switches (gate mode)
    // - Bottom: Hand cranks (duration/ratchets) - condensed
    
    int16_t leverY = y;
    int16_t leverH = 100;  // Reduced from 115
    int16_t switchY = leverY + leverH + 3;  // Reduced spacing
    int16_t switchH = 25;  // Reduced from 30
    int16_t crankY = switchY + switchH + 3;  // Reduced spacing
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
    int16_t leverWidth = (w - 6) / 8 - leverSpacing;  // 3px margins on each side
    int16_t leverStartX = x + 3;
    
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
                
                // Lever shaft (double line for depth) - centered
                int16_t shaftX = leverX + leverWidth / 2;
                display->drawLine(shaftX - 1, leverY, shaftX - 1, leverY + shaftHeight, COLOR_DIM);
                display->drawLine(shaftX, leverY, shaftX, leverY + shaftHeight, COLOR_FG);
            } else if (handleColorChanged) {
                // Only redraw handle for trigger flash - centered smaller handle
                int16_t visualHandleWidth = leverWidth * 7 / 10;  // 70% of leverWidth
                int16_t handleOffsetX = (leverWidth - visualHandleWidth) / 2;
                display->fillRect(leverX + handleOffsetX, handleY - 2, visualHandleWidth, handleHeight + 4, COLOR_BG);
            }
            
            // Lever handle (beveled rectangle) - 70% width, centered
            int16_t visualHandleWidth = leverWidth * 7 / 10;  // Reduced by 30%
            int16_t handleOffsetX = (leverWidth - visualHandleWidth) / 2;
            
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
            
            display->fillRect(leverX + handleOffsetX + 1, handleY, visualHandleWidth - 2, handleHeight, handleColor);
            // Highlight edge
            display->drawLine(leverX + handleOffsetX + 2, handleY + 1, leverX + handleOffsetX + visualHandleWidth - 3, handleY + 1, COLOR_FG);
        }
        
        // Highlight box removed - lever handle color change is sufficient for touch feedback
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
            
            // Draw toggle switch housing (vertical slot)
            int16_t slotX = switchX + switchWidth / 2 - 2;
            int16_t slotY = switchY + 3;
            int16_t slotW = 4;
            int16_t slotH = switchH - 6;
            
            // Outer housing (darker border)
            display->drawRect(slotX - 1, slotY - 1, slotW + 2, slotH + 2, COLOR_DIM);
            // Inner slot (black)
            display->fillRect(slotX, slotY, slotW, slotH, 0x0000);
            
            // Calculate toggle handle position based on gate mode
            int16_t handleY;
            uint16_t handleColor;
            
            if (gateMode == 0) {
                // NORMAL: Toggle UP (top position)
                handleY = slotY + 2;
                handleColor = COLOR_ACCENT;  // Cyan for normal/on
            } else if (gateMode == 1) {
                // SKIP: Toggle DOWN (bottom position)
                handleY = slotY + slotH - 8;
                handleColor = COLOR_DIM;  // Gray for skip/off
            } else {
                // SLIDE: Toggle MIDDLE (center position)
                handleY = slotY + slotH / 2 - 3;
                handleColor = COLOR_DIAL;  // Light blue for slide/glide
            }
            
            // Draw toggle handle (small rounded rectangle)
            int16_t handleW = switchWidth - 4;
            int16_t handleX = switchX + 2;
            int16_t handleH = 6;
            
            // Handle body
            display->fillRect(handleX, handleY, handleW, handleH, handleColor);
            // Handle highlight (top edge for 3D effect)
            display->drawLine(handleX + 1, handleY + 1, handleX + handleW - 2, handleY + 1, COLOR_FG);
            // Handle shadow (bottom edge)
            display->drawLine(handleX + 1, handleY + handleH - 1, handleX + handleW - 2, handleY + handleH - 1, 0x2104);
            
            // Highlight border removed - switch position is clear visual feedback
        }
    }
    
    // === HAND CRANKS (Duration/Ratchets) ===
    int16_t crankCenterY = crankY + 15;  // Reduced from 20
    int16_t crankRadius = 7;  // Reduced from 8
    
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
            display->drawText(crankCenterX - 2, crankY + 35, beatText, COLOR_FG, FONT_SMALL);
            
            // Show spinning animation if this is current step and ratcheting
            if (i == currentStep && duration > 1 && scriptSlots[slot].seqCurrentBeat > 0) {
                // Draw beat indicator dots
                uint8_t currentBeat = scriptSlots[slot].seqCurrentBeat;
                for (uint8_t b = 0; b < currentBeat && b < duration; b++) {
                    int16_t dotX = crankCenterX - (duration * 2) + (b * 4) + 2;
                    int16_t dotY = crankY + 45;  // Reduced from 55
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
            display->fillRect(crankCenterX - 16, crankY + 42, 32, 8, COLOR_BG);  // Adjusted position
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

// Carousel management methods
void UI::toggleCarousel(uint8_t slot) {
    if (slot < MAX_SCRIPTS) {
        scriptSlots[slot].carouselActive = !scriptSlots[slot].carouselActive;
        if (scriptSlots[slot].carouselActive) {
            // Initialize carousel position to current chord
            scriptSlots[slot].carouselSelectedIdx = scriptSlots[slot].selectedChordSlot;
        }
    }
}

void UI::rotateCarousel(uint8_t slot, int8_t delta) {
    if (slot < MAX_SCRIPTS && scriptSlots[slot].carouselActive) {
        int16_t newIdx = (int16_t)scriptSlots[slot].carouselSelectedIdx + delta;
        if (newIdx < 0) newIdx = 11;
        if (newIdx > 11) newIdx = 0;
        scriptSlots[slot].carouselSelectedIdx = (uint8_t)newIdx;
    }
}

void UI::selectFromCarousel(uint8_t slot) {
    if (slot < MAX_SCRIPTS && scriptSlots[slot].carouselActive) {
        // Apply the selected chord from carousel
        uint8_t selectedChord = scriptSlots[slot].selectedChordSlot;
        uint8_t newRoot = chordLibrary[scriptSlots[slot].carouselSelectedIdx].root;
        uint8_t newType = chordLibrary[scriptSlots[slot].carouselSelectedIdx].type;

        // Update UI state
        scriptSlots[slot].chordRoots[selectedChord] = newRoot;
        scriptSlots[slot].chordTypes[selectedChord] = newType;

        // Also update the chord in the last tracking arrays so the UI detects the change
        scriptSlots[slot].lastChordRoots[selectedChord] = 255;  // Force redraw
        scriptSlots[slot].lastChordTypes[selectedChord] = 255;  // Force redraw

        // Close carousel
        scriptSlots[slot].carouselActive = false;
        scriptSlots[slot].lastCarouselActive = true; // Force overlay clear on next draw
        // Force full redraw of chord sequencer
        scriptSlots[slot].lastCurrentChordSlot = 255;
        scriptSlots[slot].lastSelectedChordSlot = 255;
    }
}

void UI::getCarouselChords(uint8_t indices[10], uint8_t roots[10], uint8_t types[10]) const {
    // Return 10 chords (all except the current selection)
    int idx = 0;
    for (int i = 0; i < 12; i++) {
        indices[idx] = i;
        roots[idx] = chordLibrary[i].root;
        types[idx] = chordLibrary[i].type;
        idx++;
    }
}

void UI::drawChordSequencerCarouselOverlay(uint8_t slot, int16_t x, int16_t y, int16_t w, int16_t h) {
    if (!display || slot >= MAX_SCRIPTS) return;
    
    bool carouselActive = scriptSlots[slot].carouselActive;
    uint8_t selectedIdx = scriptSlots[slot].carouselSelectedIdx;
    
    // If not active and wasn't active, skip drawing
    if (!carouselActive && !scriptSlots[slot].lastCarouselActive) {
        return;
    }
    
    // If carousel is being closed, clear the entire carousel area
    if (!carouselActive && scriptSlots[slot].lastCarouselActive) {
        // Clear a wide area to remove all carousel traces
        int16_t clearX = x + 10;
        int16_t clearW = w - 20;
        display->fillRect(clearX, y - 150, clearW, 300, COLOR_BG);
        
        scriptSlots[slot].lastCarouselActive = carouselActive;
        scriptSlots[slot].carouselScrollOffset = 0;
        return;
    }
    
    // Only draw if carousel is active
    if (!carouselActive) return;
    
    const char* noteNames[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
    const char* typeNames[] = {"maj", "min"};
    char chordBuffer[16];
    
    // Get selected chord slot (clamp away from plus box)
    uint8_t chordCount = scriptSlots[slot].chordCount;
    if (chordCount == 0) chordCount = 1;
    if (chordCount > MAX_CHORD_SLOTS) chordCount = MAX_CHORD_SLOTS;
    uint8_t selectedChordSlot = scriptSlots[slot].selectedChordSlot;
    if (selectedChordSlot >= chordCount) {
        selectedChordSlot = chordCount - 1;
    }

    // Calculate total beats
    uint16_t totalBeats = 0;
    for (uint8_t i = 0; i < chordCount; i++) {
        totalBeats += scriptSlots[slot].chordBeats[i];
    }
    if (totalBeats == 0) totalBeats = 1;
    
    // Chart dimensions and positions
    int16_t chartH = 56;  // Height of position 0 box (same as progression boxes)
    int16_t chartY = y + (h - chartH) / 2;
    int16_t upperHorizon = chartY;  // Top border of selected chords
    int16_t lowerHorizon = chartY + chartH;  // Bottom border of selected chords
    int16_t chartStartX = x + 10;
    int16_t chartW = w - 20;
    
    // Calculate selected chord box X position and width
    int16_t selectedChordX = chartStartX;
    for (uint8_t i = 0; i < selectedChordSlot; i++) {
        int16_t segmentW = (chartW * scriptSlots[slot].chordBeats[i]) / totalBeats;
        selectedChordX += segmentW;
    }
    int16_t selectedChordWidth = (chartW * scriptSlots[slot].chordBeats[selectedChordSlot]) / totalBeats;
    
    // Carousel box dimensions
    int16_t outerBoxHeight = 25;  // Height of positions ±1, ±2, ±3
    int16_t centerBoxHeight = chartH;  // Height of position 0
    int16_t boxSpacing = 2;  // 2 pixels between boxes
    
    // Smooth animation: decelerate toward 0
    int16_t scrollOffset = scriptSlots[slot].carouselScrollOffset;
    if (scrollOffset != 0) {
        int16_t reduction = scrollOffset / 3;
        if (reduction == 0) reduction = (scrollOffset > 0) ? 1 : -1;
        scriptSlots[slot].carouselScrollOffset -= reduction;
        if (abs(scriptSlots[slot].carouselScrollOffset) < 2) {
            scriptSlots[slot].carouselScrollOffset = 0;
        }
    }
    scrollOffset = scriptSlots[slot].carouselScrollOffset;
    
    // Only redraw if something changed (animation happening or first draw)
    bool needsRedraw = (scrollOffset != scriptSlots[slot].lastCarouselScrollOffset) || 
                       (selectedIdx != scriptSlots[slot].lastCarouselSelectedIdx) ||
                       (scriptSlots[slot].lastCarouselScrollOffset == 0 && scriptSlots[slot].lastCarouselSelectedIdx == 255);
    
    if (!needsRedraw) {
        return;  // Nothing to update
    }
    
    // Clear only the carousel column (entire vertical strip)
    display->fillRect(selectedChordX, upperHorizon - 150, selectedChordWidth, 356, COLOR_BG);
    
    // Calculate the vertical positions of all 7 boxes
    // Position 0 must have its top at upperHorizon and bottom at lowerHorizon
    // The scroll offset shifts all boxes uniformly
    int16_t pos0Y = upperHorizon + scrollOffset;
    
    // Helper to get Y position for any position
    auto getBoxY = [&](int8_t position) -> int16_t {
        if (position == 0) return pos0Y;
        
        int16_t y_pos = pos0Y;
        if (position > 0) {
            // Positions +1, +2, +3 go below position 0
            y_pos += centerBoxHeight + boxSpacing;
            for (int8_t p = 1; p < position; p++) {
                y_pos += outerBoxHeight + boxSpacing;
            }
        } else {
            // Positions -1, -2, -3 go above position 0
            y_pos -= boxSpacing + outerBoxHeight;
            for (int8_t p = -1; p > position; p--) {
                y_pos -= (outerBoxHeight + boxSpacing);
            }
        }
        return y_pos;
    };
    
    // Helper to get box height for any position
    auto getBoxHeight = [&](int8_t position) -> int16_t {
        return (position == 0) ? centerBoxHeight : outerBoxHeight;
    };
    
    // Draw all 7 carousel boxes in fixed sequential order
    // Like a suitcase lock: if position 0 shows "5", then -1 shows "4", +1 shows "6", etc.
    for (int8_t pos = -3; pos <= 3; pos++) {
        // Always show chords in sequential order from the library
        // Position 0 shows selectedIdx, position -1 shows selectedIdx-1, position +1 shows selectedIdx+1
        int16_t chordIdx = selectedIdx + pos;
        if (chordIdx < 0) chordIdx += 12;  // Wrap negative
        if (chordIdx >= 12) chordIdx -= 12;  // Wrap positive
        
        uint8_t root = chordLibrary[chordIdx].root;
        uint8_t type = chordLibrary[chordIdx].type;
        
        int16_t boxY = getBoxY(pos);
        int16_t boxHeight = getBoxHeight(pos);
        
        // Draw rounded box
        display->fillRoundRect(selectedChordX, boxY, selectedChordWidth, boxHeight, 6, 
                              (pos == 0) ? 0x5D9F : 0x1082);
        
        // Draw text
        if (pos == 0) {
            // Center position: use same text format as progression boxes
            char rootLabel[4];
            char typeLabel[4];
            snprintf(rootLabel, sizeof(rootLabel), "%s", noteNames[root]);
            snprintf(typeLabel, sizeof(typeLabel), "%s", typeNames[type]);
            
            int16_t bxR, byR, bxT, byT; uint16_t bwR, bhR, bwT, bhT;
            display->getTextBounds(rootLabel, 0, 0, &bxR, &byR, &bwR, &bhR, FONT_MEDIUM);
            display->getTextBounds(typeLabel, 0, 0, &bxT, &byT, &bwT, &bhT, FONT_SMALL);
            
            int16_t totalW = bwR + 2 + bwT;
            int16_t textX = selectedChordX + (selectedChordWidth - totalW) / 2 - bxR;
            int16_t textY = boxY + (boxHeight - std::max((uint16_t)bhR, (uint16_t)bhT)) / 2 - std::min(byR, byT);
            
            display->drawText(textX, textY, rootLabel, COLOR_BG, FONT_MEDIUM);
            display->drawText(textX + bwR + 2 - bxT, textY, typeLabel, COLOR_BG, FONT_SMALL);
        } else {
            // Outer positions: smaller text, centered
            snprintf(chordBuffer, sizeof(chordBuffer), "%s %s", noteNames[root], typeNames[type]);
            int16_t textY = boxY + boxHeight / 2;
            display->drawTextCentered(textY, chordBuffer, COLOR_DIM, FONT_SMALL);
        }
    }
    
    // Update tracking variables
    scriptSlots[slot].lastCarouselActive = carouselActive;
    scriptSlots[slot].lastCarouselScrollOffset = scrollOffset;
    scriptSlots[slot].lastCarouselSelectedIdx = selectedIdx;
}

// Full-screen chord list overlay
void UI::openChordList(uint8_t slot, uint8_t targetSlot) {
    if (slot >= MAX_SCRIPTS) return;

    uint8_t chordCount = scriptSlots[slot].chordCount;
    if (chordCount > MAX_CHORD_SLOTS) chordCount = MAX_CHORD_SLOTS;
    scriptSlots[slot].chordListActive = true;
    scriptSlots[slot].lastChordListActive = false; // Force draw
    scriptSlots[slot].chordListTargetSlot = targetSlot;
    
    // Rank chords using the ranking engine (Package 4)
    rankChordsForDisplay(slot);

    // Start selection on current chord value when editing an existing slot
    uint8_t startRoot = 0;
    uint8_t startType = 0;
    if (targetSlot < chordCount) {
        startRoot = scriptSlots[slot].chordRoots[targetSlot];
        startType = scriptSlots[slot].chordTypes[targetSlot];
    }

    uint8_t matchedIdx = 0;
    for (uint8_t i = 0; i < 12; i++) {
        if (chordLibrary[i].root == startRoot && chordLibrary[i].type == startType) {
            matchedIdx = i;
            break;
        }
    }
    scriptSlots[slot].chordListSelectedIdx = matchedIdx;
    scriptSlots[slot].lastChordListSelectedIdx = 255; // Force selection draw
}

void UI::navigateChordList(uint8_t slot, int8_t delta) {
    if (slot >= MAX_SCRIPTS) return;
    if (!scriptSlots[slot].chordListActive) return;

    int16_t idx = (int16_t)scriptSlots[slot].chordListSelectedIdx + delta;
    while (idx < 0) idx += 12;
    while (idx >= 12) idx -= 12;
    scriptSlots[slot].chordListSelectedIdx = (uint8_t)idx;
}

uint8_t UI::selectFromChordList(uint8_t slot) {
    if (slot >= MAX_SCRIPTS) return 255;
    if (!scriptSlots[slot].chordListActive) return 255;

    uint8_t chordCount = scriptSlots[slot].chordCount;
    if (chordCount > MAX_CHORD_SLOTS) chordCount = MAX_CHORD_SLOTS;
    uint8_t target = scriptSlots[slot].chordListTargetSlot;
    if (target > chordCount) target = chordCount; // Safeguard for add slot

    uint8_t root, type;
    
    // Get chord from ranked or static list
    if (scriptSlots[slot].rankedChordsValid && scriptSlots[slot].rankedChordCount > 0) {
        uint8_t selectedIdx = scriptSlots[slot].chordListSelectedIdx;
        if (selectedIdx >= scriptSlots[slot].rankedChordCount) selectedIdx = 0;
        root = scriptSlots[slot].rankedChords[selectedIdx].rootNote;
        type = (uint8_t)scriptSlots[slot].rankedChords[selectedIdx].type;
    } else {
        uint8_t selectedIdx = scriptSlots[slot].chordListSelectedIdx;
        if (selectedIdx >= 12) selectedIdx = 0;  // Safety bounds check
        root = chordLibrary[selectedIdx].root;
        type = chordLibrary[selectedIdx].type;
    }

    // If targeting the plus box, append a new chord when there's room; otherwise edit the last slot
    if (target >= chordCount) {
        if (chordCount < MAX_CHORD_SLOTS) {
            scriptSlots[slot].chordCount = chordCount + 1;
            target = chordCount;
            scriptSlots[slot].chordBeats[target] = 8; // Default beat length for new chords
        } else {
            target = (chordCount > 0) ? (chordCount - 1) : 0;
        }
    }

    if (target < MAX_CHORD_SLOTS) {
        scriptSlots[slot].chordRoots[target] = root;
        scriptSlots[slot].chordTypes[target] = type;
        scriptSlots[slot].lastChordRoots[target] = 255; // Force redraw detection
        scriptSlots[slot].lastChordTypes[target] = 255;
        scriptSlots[slot].selectedChordSlot = target;
    }

    // Close overlay and force redraw
    scriptSlots[slot].chordListActive = false;
    scriptSlots[slot].lastChordListActive = true; // Ensure clear
    scriptSlots[slot].lastChordListSelectedIdx = 255;
    scriptSlots[slot].lastSelectedChordSlot = 255;
    scriptSlots[slot].lastCurrentChordSlot = 255;
    scriptSlots[slot].lastChordBeatCounter = 255;

    return target;
}

void UI::rankChordsForDisplay(uint8_t slot) {
    if (slot >= MAX_SCRIPTS || !scriptManager) {
        scriptSlots[slot].rankedChordsValid = false;
        return;
    }
    
    // Request ranked chords from script manager
    // This invokes the ranking engine internally
    scriptSlots[slot].rankedChordCount = scriptManager->rankChordsForSequencer(
        slot, 
        scriptSlots[slot].rankedChords, 
        24
    );
    
    scriptSlots[slot].rankedChordsValid = (scriptSlots[slot].rankedChordCount > 0);
}

void UI::drawChordListOverlay(uint8_t slot, int16_t x, int16_t y, int16_t w, int16_t h) {
    if (!display || slot >= MAX_SCRIPTS) return;

    bool active = scriptSlots[slot].chordListActive;
    bool wasActive = scriptSlots[slot].lastChordListActive;
    uint8_t selectedIdx = scriptSlots[slot].chordListSelectedIdx;
    uint8_t lastSelectedIdx = scriptSlots[slot].lastChordListSelectedIdx;

    // If overlay is inactive, just update tracking and exit
    if (!active) {
        scriptSlots[slot].lastChordListActive = false;
        return;
    }

    // Overlay geometry
    int16_t overlayX = x + 4;
    int16_t overlayW = w - 8;
    int16_t overlayY = y - 10;
    int16_t overlayH = h + 20;
    if (overlayY < 0) overlayY = 0;
    if (overlayY + overlayH > SCREEN_HEIGHT) overlayH = SCREEN_HEIGHT - overlayY;

    const char* noteNames[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
    const char* typeNames[] = {"maj", "min"};

    bool firstDraw = !wasActive;
    bool selectionChanged = (selectedIdx != lastSelectedIdx);

    // Redraw entire overlay on first draw
    if (firstDraw) {
        display->fillRect(overlayX, overlayY, overlayW, overlayH, COLOR_BG);
        
        // Show title with ranking status
        const char* title = scriptSlots[slot].rankedChordsValid ? "ranked chords" : "chord library";
        display->drawText(overlayX + 6, overlayY + 4, title, COLOR_DIM, FONT_SMALL);
    }

    // Grid layout: 3 columns x 4 rows (12 chords visible)
    const uint8_t cols = 3;
    const uint8_t rows = 4;
    int16_t cellGap = 4;
    int16_t cellW = (overlayW - (cols + 1) * cellGap) / cols;
    int16_t cellH = 26;
    int16_t startY = overlayY + 18;
    
    // Determine which chord list to use
    bool useRanked = scriptSlots[slot].rankedChordsValid && scriptSlots[slot].rankedChordCount > 0;
    uint8_t displayCount = useRanked ? scriptSlots[slot].rankedChordCount : 12;
    if (displayCount > 12) displayCount = 12;  // Show only 12 at a time

    auto drawCell = [&](uint8_t displayIdx, bool selected) {
        // Get chord from ranked or static list
        uint8_t chordRoot, chordType;
        float score = 0.0f;
        
        if (useRanked && displayIdx < scriptSlots[slot].rankedChordCount) {
            chordRoot = scriptSlots[slot].rankedChords[displayIdx].rootNote;
            chordType = scriptSlots[slot].rankedChords[displayIdx].type;
            score = scriptSlots[slot].rankedChords[displayIdx].totalScore;
        } else if (!useRanked && displayIdx < 12) {
            chordRoot = chordLibrary[displayIdx].root;
            chordType = chordLibrary[displayIdx].type;
            score = 0.5f;  // Neutral score for static list
        } else {
            return;  // Out of range
        }
        
        uint8_t col = displayIdx % cols;
        uint8_t row = displayIdx / cols;
        int16_t cellX = overlayX + cellGap + col * (cellW + cellGap);
        int16_t cellY = startY + row * (cellH + cellGap);
        
        // Color based on selection and ranking score
        uint16_t bg;
        uint16_t textColor;
        
        if (selected) {
            bg = COLOR_ACCENT;        // Cyan for selected
            textColor = COLOR_BG;
        } else if (useRanked) {
            // Visual de-emphasis based on score
            if (score >= 0.8f) {
                bg = 0x2945;  // Brighter gray for top-ranked
                textColor = COLOR_FG;
            } else if (score >= 0.6f) {
                bg = 0x1082;  // Medium gray for mid-ranked
                textColor = COLOR_DIM;
            } else {
                bg = 0x0841;  // Darker gray for low-ranked
                textColor = 0x39E7;  // Dimmer text
            }
        } else {
            bg = 0x1082;        // Standard gray for static list
            textColor = COLOR_FG;
        }
        
        display->fillRoundRect(cellX, cellY, cellW, cellH, 5, bg);

        char label[8];
        snprintf(label, sizeof(label), "%s %s", noteNames[chordRoot], typeNames[chordType]);
        int16_t bx, by; uint16_t bw, bh;
        display->getTextBounds(label, 0, 0, &bx, &by, &bw, &bh, FONT_SMALL);
        int16_t textX = cellX + (cellW - bw) / 2 - bx;
        int16_t textY = cellY + (cellH - bh) / 2 - by;
        display->drawText(textX, textY, label, textColor, FONT_SMALL);
    };

    if (firstDraw) {
        for (uint8_t i = 0; i < displayCount; i++) {
            drawCell(i, i == selectedIdx);
        }
    } else if (selectionChanged) {
        if (lastSelectedIdx < displayCount) {
            drawCell(lastSelectedIdx, false);
        }
        if (selectedIdx < displayCount) {
            drawCell(selectedIdx, true);
        }
    }

    scriptSlots[slot].lastChordListActive = active;
    scriptSlots[slot].lastChordListSelectedIdx = selectedIdx;
}

void UI::openBeatCountPicker(uint8_t slot, uint8_t initialBeats) {
    if (slot >= MAX_SCRIPTS) return;
    
    scriptSlots[slot].beatCountPickerActive = true;
    scriptSlots[slot].lastBeatCountPickerActive = false; // Force draw
    
    if (initialBeats < 1) initialBeats = 1;
    if (initialBeats > 32) initialBeats = 32;
    scriptSlots[slot].beatCountSelection = initialBeats;
    scriptSlots[slot].lastBeatCountSelection = 255; // Force first draw
    
    // Load current per-chord parameters from the selected chord
    uint8_t selectedChordSlot = scriptSlots[slot].selectedChordSlot;
    uint8_t chordCount = scriptSlots[slot].chordCount;
    if (selectedChordSlot >= chordCount && chordCount > 0) selectedChordSlot = chordCount - 1;
    
    // Initialize selected parameter to beat count, editing off
    scriptSlots[slot].selectedChordParam = 0;  // Start with beat count
    scriptSlots[slot].editingChordParam = false;
    scriptSlots[slot].lastSelectedChordParam = 255;  // Force first draw
    scriptSlots[slot].lastEditingChordParam = false;
    
    // Sync current chord parameters from script
    syncChordParamsFromScript(slot, selectedChordSlot);
}

void UI::syncChordParamsFromScript(uint8_t slot, uint8_t chordSlot) {
    if (slot >= MAX_SCRIPTS || !scriptManager) return;
    
    // Get per-chord parameters from script manager (which queries the ChordSequencerScript)
    // This is called when opening the Beat Count Picker to load current values
    uint8_t inversion = scriptManager->getChordSequencerChordInversion(slot, chordSlot);
    float spread = scriptManager->getChordSequencerChordSpread(slot, chordSlot);
    uint8_t theoryMode = scriptManager->getChordSequencerChordTheoryMode(slot, chordSlot);
    
    scriptSlots[slot].chordInversion = inversion;
    scriptSlots[slot].chordSpread = spread;
    scriptSlots[slot].chordTheoryMode = theoryMode;
    scriptSlots[slot].lastChordInversion = 255;  // Force redraw
    scriptSlots[slot].lastChordSpread = -999.0f;  // Force redraw
    scriptSlots[slot].lastChordTheoryMode = 255;  // Force redraw
}

void UI::navigateBeatCountPicker(uint8_t slot, int8_t delta) {
    if (slot >= MAX_SCRIPTS) return;
    if (!scriptSlots[slot].beatCountPickerActive) return;
    
    // Cyclic navigation through 5 parameters: beat count -> inversion -> spread -> theory -> done
    uint8_t currentSelection = scriptSlots[slot].selectedChordParam;
    int16_t newSelection = (int16_t)currentSelection + delta;
    if (newSelection < 0) newSelection = 4;  // Wrap to done
    if (newSelection > 4) newSelection = 0;  // Wrap to beats
    scriptSlots[slot].selectedChordParam = (uint8_t)newSelection;
}

void UI::finalizeBeatCountPicker(uint8_t slot) {
    if (slot >= MAX_SCRIPTS) return;
    if (!scriptSlots[slot].beatCountPickerActive) return;
    
    // Save any pending changes if still editing
    if (scriptSlots[slot].editingChordParam) {
        exitChordParamEdit(slot, true);  // Save changes and exit edit mode
    }
    
    // Close the beat count picker overlay
    scriptSlots[slot].beatCountPickerActive = false;
}

uint8_t UI::confirmBeatCount(uint8_t slot) {
    if (slot >= MAX_SCRIPTS) return 0;
    if (!scriptSlots[slot].beatCountPickerActive) return 0;
    
    uint8_t targetSlot = scriptSlots[slot].chordListTargetSlot;
    uint8_t beatCount = scriptSlots[slot].beatCountSelection;
    
    // Apply beat count to the chord
    if (targetSlot < MAX_CHORD_SLOTS) {
        scriptSlots[slot].chordBeats[targetSlot] = beatCount;
        scriptSlots[slot].lastChordBeats[targetSlot] = 255;  // Force redraw
    }
    
    // Close picker and force UI redraw
    scriptSlots[slot].beatCountPickerActive = false;
    scriptSlots[slot].lastBeatCountPickerActive = true;
    // Force full redraw of chord strip and selection after overlay
    scriptSlots[slot].lastChordCount = 255;
    scriptSlots[slot].lastSelectedChordSlot = 255;
    scriptSlots[slot].lastCurrentChordSlot = 255;
    scriptSlots[slot].lastChordBeatCounter = 255;
    
    return beatCount;
}

void UI::drawBeatCountPickerOverlay(uint8_t slot, int16_t x, int16_t y, int16_t w, int16_t h) {
    if (!display || slot >= MAX_SCRIPTS) return;
    
    bool active = scriptSlots[slot].beatCountPickerActive;
    bool wasActive = scriptSlots[slot].lastBeatCountPickerActive;
    
    // If overlay just closed, force base UI redraw next frame (do not clear here or we wipe the base)
    if (!active && wasActive) {
        scriptSlots[slot].lastBeatCountPickerActive = false;
        scriptSlots[slot].lastCurrentChordSlot = 255;
        scriptSlots[slot].lastChordBeatCounter = 255;
        scriptSlots[slot].lastSelectedChordSlot = 255;
        scriptSlots[slot].lastChordCount = 255;
        return;
    }
    
    // If not active and wasn't active, skip
    if (!active) return;
    
    // Overlay geometry
    int16_t overlayX = x + 15;
    int16_t overlayW = w - 30;
    int16_t overlayY = y + 25;
    int16_t overlayH = h - 50;
    
    bool firstDraw = !wasActive;
    
    // Layout: Two rows - beats on top (centered, smaller), inversion/spread/theory on bottom
    const char* labels[] = {"beats", "inversion", "spread", "theory"};
    
    // Bottom row: inversion, spread, theory (3 boxes) - 10% less wide, 10% less tall
    int16_t bottomBoxW = ((overlayW - 20) / 3) * 0.9;  // 3 boxes with gaps, 10% less wide
    int16_t bottomBoxH = 45;  // 10% reduction from 50: 50 * 0.9 = 45
    int16_t bottomBoxGap = 10;
    int16_t bottomBoxStartX = overlayX + 10;  // Left edge of first bottom box
    
    // Top row: beats button - spans from left edge of inversion to right edge of theory
    int16_t firstBottomBoxX = bottomBoxStartX;
    int16_t lastBottomBoxX = bottomBoxStartX + 2 * (bottomBoxW + bottomBoxGap);
    int16_t topBoxW = (lastBottomBoxX + bottomBoxW) - firstBottomBoxX;  // Span full width
    int16_t topBoxH = 45;   // 10% reduction from 50: 50 * 0.9 = 45
    int16_t topBoxX = firstBottomBoxX;  // Align with bottom boxes
    int16_t topBoxY = overlayY + 25;
    
    int16_t bottomBoxY = topBoxY + topBoxH + 10;  // Below beats box with spacing
    
    // Get current and previous chord parameters for comparison
    uint8_t selectedChordSlot = scriptSlots[slot].selectedChordSlot;
    uint8_t chordCount = scriptSlots[slot].chordCount;
    if (selectedChordSlot >= chordCount) selectedChordSlot = chordCount - 1;  // Safety
    
    // Draw title
    if (firstDraw) {
        display->fillRect(overlayX, overlayY, overlayW, overlayH, 0x1082);
        display->drawRect(overlayX, overlayY, overlayW, overlayH, COLOR_ACCENT);
        display->drawText(overlayX + 10, overlayY + 5, "chord parameters", COLOR_DIM, FONT_SMALL);
    }
    
    // Values to display
    // When editing beat count, show the beatCountSelection; otherwise show the saved value
    uint8_t beatCount;
    if (scriptSlots[slot].editingChordParam && scriptSlots[slot].selectedChordParam == 0) {
        beatCount = scriptSlots[slot].beatCountSelection;  // Show value being edited
    } else {
        beatCount = scriptSlots[slot].chordBeats[selectedChordSlot];  // Show saved value
    }
    uint8_t inversion = scriptSlots[slot].chordInversion;
    float spread = scriptSlots[slot].chordSpread;
    uint8_t theoryMode = scriptSlots[slot].chordTheoryMode;
    
    // Theory mode names for display
    static const char* theoryModeNames[] = {"func", "diat", "moda", "chro", "all"};
    
    // Draw beats box (top row, parameter 0)
    {
        int i = 0;
        bool isSelected = (scriptSlots[slot].selectedChordParam == i);
        bool isEditing = isSelected && scriptSlots[slot].editingChordParam;
        
        // Determine colors
        uint16_t boxColor;
        uint16_t textColor;
        if (isEditing) {
            boxColor = COLOR_HIGHLIGHT;  // Yellow
            textColor = COLOR_BG;
        } else if (isSelected) {
            boxColor = COLOR_ACCENT;  // Cyan
            textColor = COLOR_BG;
        } else {
            boxColor = 0x0820;  // Dark gray
            textColor = COLOR_FG;
        }
        
        // Check what changed
        bool needsRedraw = firstDraw || isSelected != (scriptSlots[slot].lastSelectedChordParam == i) ||
                          isEditing != (scriptSlots[slot].lastEditingChordParam && scriptSlots[slot].lastSelectedChordParam == i);
        
        // Check if parameter values changed
        uint8_t lastBeatCount;
        if (scriptSlots[slot].lastEditingChordParam && scriptSlots[slot].lastSelectedChordParam == 0) {
            lastBeatCount = scriptSlots[slot].lastBeatCountSelection;
        } else {
            lastBeatCount = scriptSlots[slot].chordBeats[selectedChordSlot];
        }
        if (beatCount != lastBeatCount) needsRedraw = true;
        
        if (needsRedraw) {
            // Draw box
            display->fillRoundRect(topBoxX, topBoxY, topBoxW, topBoxH, 4, boxColor);
            
            // Draw label
            display->drawText(topBoxX + 8, topBoxY + 5, labels[i], textColor, FONT_SMALL);
            
            // Draw value - much larger, centered vertically
            char valueStr[12];
            snprintf(valueStr, sizeof(valueStr), "%d", beatCount);
            
            int16_t bx, by; uint16_t bw, bh;
            display->getTextBounds(valueStr, 0, 0, &bx, &by, &bw, &bh, FONT_LARGE);
            int16_t valueX = topBoxX + (topBoxW - bw) / 2 - bx;
            int16_t valueY = topBoxY + (topBoxH - bh) / 2 - by;  // Vertical center
            display->drawText(valueX, valueY, valueStr, textColor, FONT_LARGE);
        }
    }
    
    // Draw 3 parameter boxes on bottom row (inversion, spread, theory)
    for (int i = 1; i < 4; i++) {
        int16_t boxIdx = i - 1;  // 0-2 for positioning
        int16_t boxX = bottomBoxStartX + boxIdx * (bottomBoxW + bottomBoxGap);
        
        bool isSelected = (scriptSlots[slot].selectedChordParam == i);
        bool isEditing = isSelected && scriptSlots[slot].editingChordParam;
        
        // Determine colors
        uint16_t boxColor;
        uint16_t textColor;
        if (isEditing) {
            boxColor = COLOR_HIGHLIGHT;  // Yellow
            textColor = COLOR_BG;
        } else if (isSelected) {
            boxColor = COLOR_ACCENT;  // Cyan
            textColor = COLOR_BG;
        } else {
            boxColor = 0x0820;  // Dark gray
            textColor = COLOR_FG;
        }
        
        // Check what changed
        bool needsRedraw = firstDraw || isSelected != (scriptSlots[slot].lastSelectedChordParam == i) ||
                          isEditing != (scriptSlots[slot].lastEditingChordParam && scriptSlots[slot].lastSelectedChordParam == i);
        
        // Also check if parameter values changed
        if (i == 1 && inversion != scriptSlots[slot].lastChordInversion) needsRedraw = true;
        if (i == 2 && fabs(spread - scriptSlots[slot].lastChordSpread) > 0.001f) needsRedraw = true;
        if (i == 3 && theoryMode != scriptSlots[slot].lastChordTheoryMode) needsRedraw = true;
        
        if (!needsRedraw) continue;
        
        // Draw box
        display->fillRoundRect(boxX, bottomBoxY, bottomBoxW, bottomBoxH, 4, boxColor);
        
        // Draw label
        display->drawText(boxX + 4, bottomBoxY + 5, labels[i], textColor, FONT_SMALL);
        
        // Draw value
        char valueStr[12];
        if (i == 1) {  // Inversion
            if (inversion == 255) {
                snprintf(valueStr, sizeof(valueStr), "auto");
            } else {
                snprintf(valueStr, sizeof(valueStr), "%d", inversion);
            }
        } else if (i == 2) {  // Spread
            if (spread < -0.5f) {
                snprintf(valueStr, sizeof(valueStr), "auto");
            } else {
                snprintf(valueStr, sizeof(valueStr), "%.2f", spread);
            }
        } else {  // Theory mode
            if (theoryMode == 255) {
                snprintf(valueStr, sizeof(valueStr), "auto");
            } else if (theoryMode < 5) {
                snprintf(valueStr, sizeof(valueStr), "%s", theoryModeNames[theoryMode]);
            } else {
                snprintf(valueStr, sizeof(valueStr), "???");
            }
        }
        
        int16_t bx, by; uint16_t bw, bh;
        display->getTextBounds(valueStr, 0, 0, &bx, &by, &bw, &bh, FONT_SMALL);
        int16_t valueX = boxX + (bottomBoxW - bw) / 2 - bx;
        int16_t valueY = bottomBoxY + 30;
        display->drawText(valueX, valueY, valueStr, textColor, FONT_SMALL);
    }
    
    // Draw "Done" button in bottom right of white box
    int16_t doneButtonX = overlayX + overlayW - 60;
    int16_t doneButtonY = overlayY + overlayH - 20;
    int16_t doneButtonW = 50;
    int16_t doneButtonH = 15;
    
    bool isDoneSelected = (scriptSlots[slot].selectedChordParam == 4);
    uint16_t doneColor = isDoneSelected ? COLOR_ACCENT : 0x1082;  // Cyan when selected, dark gray otherwise
    uint16_t doneTextColor = isDoneSelected ? COLOR_BG : COLOR_FG;
    
    display->fillRoundRect(doneButtonX, doneButtonY, doneButtonW, doneButtonH, 3, doneColor);
    display->drawText(doneButtonX + 8, doneButtonY + 3, "done", doneTextColor, FONT_SMALL);
    
    // Draw navigation hint at the very bottom of screen (outside white box)
    int16_t hintY = SCREEN_HEIGHT - 15;
    display->drawText(10, hintY, "turn: select, OK: edit", COLOR_DIM, FONT_SMALL);
    
    // Update last values
    scriptSlots[slot].lastBeatCountSelection = beatCount;
    scriptSlots[slot].lastSelectedChordParam = scriptSlots[slot].selectedChordParam;
    scriptSlots[slot].lastEditingChordParam = scriptSlots[slot].editingChordParam;
    scriptSlots[slot].lastChordInversion = inversion;
    scriptSlots[slot].lastChordSpread = spread;
    scriptSlots[slot].lastChordTheoryMode = theoryMode;
    scriptSlots[slot].lastBeatCountPickerActive = active;
}

void UI::rotateCarouselWithAnimation(uint8_t slot, int8_t delta) {
    if (slot < MAX_SCRIPTS && scriptSlots[slot].carouselActive) {
        // Update the carousel selection
        int16_t newIdx = (int16_t)scriptSlots[slot].carouselSelectedIdx + delta;
        if (newIdx < 0) newIdx = 11;
        if (newIdx > 11) newIdx = 0;
        scriptSlots[slot].carouselSelectedIdx = (uint8_t)newIdx;
        
        // Set animation offset: each box is 25px tall + 2px spacing = 27px per position
        // Positive delta: scroll up (negative offset makes content appear to move up)
        // Negative delta: scroll down (positive offset makes content appear to move down)
        scriptSlots[slot].carouselScrollOffset -= delta * 27;  // 25px box + 2px spacing
    }
}

void UI::drawChordCarousel(uint8_t slot) {
    if (!display || slot >= MAX_SCRIPTS || !scriptSlots[slot].carouselActive) return;
    
    const char* noteNames[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
    const char* typeNames[] = {"maj", "min"};
    char chordBuffer[16];  // Local buffer for chord display
    
    uint8_t selectedIdx = scriptSlots[slot].carouselSelectedIdx;
    
    // Carousel dimensions (mimicking iOS album selector)
    int16_t carouselCenterX = 160;  // Center of screen
    int16_t carouselCenterY = 120;  // Center of screen
    int16_t itemHeight = 40;
    
    // Draw semi-transparent overlay
    display->fillRect(0, 0, 320, 240, 0x0000);  // Black overlay
    
    // Draw carousel items: 2 above, center (selected), 2 below
    // Calculate visible indices (wrapping)
    uint8_t visibleIndices[5];
    for (int i = -2; i <= 2; i++) {
        int idx = (selectedIdx + i);
        if (idx < 0) idx += 12;
        if (idx >= 12) idx -= 12;
        visibleIndices[i + 2] = idx;
    }
    
    // Draw the 5 visible items
    for (int pos = -2; pos <= 2; pos++) {
        uint8_t chordIdx = visibleIndices[pos + 2];
        uint8_t root = chordLibrary[chordIdx].root;
        uint8_t type = chordLibrary[chordIdx].type;
        
        int16_t itemY = carouselCenterY + (pos * itemHeight);
        bool isCenterItem = (pos == 0);
        
        // Scale and alpha based on position
        uint16_t itemWidth = isCenterItem ? 140 : 100;
        uint16_t itemX = carouselCenterX - itemWidth / 2;
        uint16_t bgColor = isCenterItem ? 0x2104 : 0x1082;  // Darker boxes for non-center
        
        // Draw rounded box
        display->fillRoundRect(itemX, itemY - itemHeight/2 + 5, itemWidth, itemHeight - 10, 8, bgColor);
        
        // Draw chord text (root + type)
        snprintf(chordBuffer, sizeof(chordBuffer), "%s %s", noteNames[root], typeNames[type]);
        
        int16_t textY = itemY;
        
        if (isCenterItem) {
            display->drawTextCentered(textY - 5, chordBuffer, COLOR_ACCENT, FONT_MEDIUM);
            // Draw highlight border around center item
            display->drawRoundRect(itemX, itemY - itemHeight/2 + 5, itemWidth, itemHeight - 10, 8, COLOR_ACCENT);
        } else {
            display->drawTextCentered(textY - 5, chordBuffer, COLOR_DIM, FONT_SMALL);
        }
    }
    
    // Draw hint text
    display->drawTextCentered(210, "ENCODER: SELECT | BUTTON: APPLY", COLOR_DIM, FONT_SMALL);
}

// ============================================================================
// Per-Chord Parameter Methods
// ============================================================================

void UI::enterChordParamEdit(uint8_t slot) {
    if (slot >= MAX_SCRIPTS) return;
    scriptSlots[slot].editingChordParam = true;
}

void UI::exitChordParamEdit(uint8_t slot, bool save) {
    if (slot >= MAX_SCRIPTS) return;
    
    if (save) {
        // Save changes to script
        uint8_t selectedChordSlot = scriptSlots[slot].selectedChordSlot;
        uint8_t chordCount = scriptSlots[slot].chordCount;
        if (selectedChordSlot >= chordCount && chordCount > 0) selectedChordSlot = chordCount - 1;
        
        uint8_t param = scriptSlots[slot].selectedChordParam;
        
        if (!scriptManager) return;  // Safety check
        
        switch (param) {
            case 0: // Beat count
                scriptManager->setChordSequencerChordBeats(slot, selectedChordSlot, scriptSlots[slot].beatCountSelection);
                break;
            case 1: // Inversion
                scriptManager->setChordSequencerChordInversion(slot, selectedChordSlot, scriptSlots[slot].chordInversion);
                break;
            case 2: // Spread
                scriptManager->setChordSequencerChordSpread(slot, selectedChordSlot, scriptSlots[slot].chordSpread);
                break;
            case 3: // Theory Mode
                scriptManager->setChordSequencerChordTheoryMode(slot, selectedChordSlot, scriptSlots[slot].chordTheoryMode);
                break;
        }
    }
    
    scriptSlots[slot].editingChordParam = false;
    scriptSlots[slot].chordParamEditJustExited = true;  // Flag that we just exited
}

void UI::adjustChordParam(uint8_t slot, int8_t delta) {
    if (slot >= MAX_SCRIPTS || !scriptSlots[slot].editingChordParam) return;
    
    uint8_t param = scriptSlots[slot].selectedChordParam;
    
    switch (param) {
        case 0: // Beat count
            {
                int16_t newBeats = (int16_t)scriptSlots[slot].beatCountSelection + delta;
                if (newBeats < 1) newBeats = 32;
                if (newBeats > 32) newBeats = 1;
                scriptSlots[slot].beatCountSelection = (uint8_t)newBeats;
            }
            break;
            
        case 1: // Inversion
            {
                int16_t newInversion = (int16_t)scriptSlots[slot].chordInversion + delta;
                if (newInversion < 0) newInversion = 255;  // Cycle to auto
                if (newInversion > 255) newInversion = 0;  // Cycle from auto to 0
                if (newInversion == 255) {
                    scriptSlots[slot].chordInversion = 255;  // Auto
                } else if (newInversion > 2) {
                    newInversion = 2;  // Max 3 inversions (0, 1, 2)
                    scriptSlots[slot].chordInversion = (uint8_t)newInversion;
                } else {
                    scriptSlots[slot].chordInversion = (uint8_t)newInversion;
                }
            }
            break;
            
        case 2: // Spread (0.0-1.0, or -1.0 for auto)
            {
                float newSpread = scriptSlots[slot].chordSpread + (delta * 0.1f);
                // Cycle: auto (-1.0) -> 0.0 -> 1.0 -> auto
                if (newSpread < -0.95f) {
                    // Currently in auto range, moving negative wraps to max
                    newSpread = 1.0f;
                } else if (newSpread > 1.05f) {
                    // Past max, wrap to auto
                    newSpread = -1.0f;
                }
                scriptSlots[slot].chordSpread = constrain(newSpread, -1.0f, 1.0f);
            }
            break;
            
        case 3: // Theory Mode
            {
                int16_t newMode = (int16_t)scriptSlots[slot].chordTheoryMode + delta;
                if (newMode < 0) newMode = 255;  // Cycle to auto
                if (newMode > 255) newMode = 0;  // Cycle from auto to 0
                if (newMode == 255) {
                    scriptSlots[slot].chordTheoryMode = 255;  // Auto
                } else if (newMode > 4) {
                    newMode = 4;
                    scriptSlots[slot].chordTheoryMode = (uint8_t)newMode;
                } else {
                    scriptSlots[slot].chordTheoryMode = (uint8_t)newMode;
                }
            }
            break;
    }
}

// ============================================================================
// Global Parameter Methods
// ============================================================================


void UI::enterGlobalParamEdit(uint8_t slot) {
    if (slot >= MAX_SCRIPTS) return;
    scriptSlots[slot].editingGlobalParam = true;
}

void UI::exitGlobalParamEdit(uint8_t slot, bool save) {
    if (slot >= MAX_SCRIPTS) return;
    
    if (!save) {
        // Restore previous values - will be synced from script manager on next update
        scriptSlots[slot].lastGlobalKey = 255;  // Force re-sync
    }
    
    scriptSlots[slot].editingGlobalParam = false;
}

void UI::adjustGlobalParam(uint8_t slot, int8_t delta) {
    if (slot >= MAX_SCRIPTS || !scriptSlots[slot].editingGlobalParam) return;
    
    uint8_t param = scriptSlots[slot].selectedGlobalParam;
    
    switch (param) {
        case 0: // Key
            {
                int16_t newKey = (int16_t)scriptSlots[slot].globalKey + delta;
                if (newKey < 0) newKey = 11;
                if (newKey > 11) newKey = 0;
                scriptSlots[slot].globalKey = (uint8_t)newKey;
            }
            break;
            
        case 1: // Degree
            {
                int16_t newDegree = (int16_t)scriptSlots[slot].globalDegree + delta;
                if (newDegree < 0) newDegree = 6;  // DEGREE_LOCRIAN
                if (newDegree > 6) newDegree = 0;  // DEGREE_MAJOR
                scriptSlots[slot].globalDegree = (uint8_t)newDegree;
            }
            break;
            
        case 2: // Theory Mode
            {
                int16_t newMode = (int16_t)scriptSlots[slot].globalTheoryMode + delta;
                if (newMode < 0) newMode = 4;  // THEORY_ALL
                if (newMode > 4) newMode = 0;  // THEORY_FUNCTIONAL
                scriptSlots[slot].globalTheoryMode = (uint8_t)newMode;
            }
            break;
            
        case 3: // Voice Leading Compactness
            {
                float newValue = scriptSlots[slot].globalVoiceLeading + (delta * 0.05f);
                if (newValue < 0.0f) newValue = 0.0f;
                if (newValue > 1.0f) newValue = 1.0f;
                scriptSlots[slot].globalVoiceLeading = newValue;
            }
            break;
            
        case 4: // Energy
            {
                float newValue = scriptSlots[slot].globalEnergy + (delta * 0.05f);
                if (newValue < 0.0f) newValue = 0.0f;
                if (newValue > 1.0f) newValue = 1.0f;
                scriptSlots[slot].globalEnergy = newValue;
            }
            break;
    }
}

void UI::syncGlobalsFromScript(uint8_t slot, const GlobalParameters& globals) {
    if (slot >= MAX_SCRIPTS) return;
    
    // Detect actual changes in the source script
    uint8_t newKey = (uint8_t)globals.key;
    uint8_t newDegree = (uint8_t)globals.degree;
    uint8_t newTheoryMode = (uint8_t)globals.theoryMode;
    float newVoiceLeading = globals.voiceLeadingCompactness;
    float newEnergy = globals.energy;
    
    // Only update if something actually changed, preserving last* tracking
    if (newKey != scriptSlots[slot].globalKey) {
        scriptSlots[slot].globalKey = newKey;
    }
    if (newDegree != scriptSlots[slot].globalDegree) {
        scriptSlots[slot].globalDegree = newDegree;
    }
    if (newTheoryMode != scriptSlots[slot].globalTheoryMode) {
        scriptSlots[slot].globalTheoryMode = newTheoryMode;
    }
    if (newVoiceLeading != scriptSlots[slot].globalVoiceLeading) {
        scriptSlots[slot].globalVoiceLeading = newVoiceLeading;
    }
    if (newEnergy != scriptSlots[slot].globalEnergy) {
        scriptSlots[slot].globalEnergy = newEnergy;
    }
}

