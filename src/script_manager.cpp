/**
 * Script Manager Implementation
 * 
 * Handles concurrent script execution similar to Norns Shield
 * 
 * Note: This is a stub implementation. In a full implementation,
 * this would integrate with SuperCollider or a Lua interpreter
 * similar to the Norns environment.
 */

#include "script_manager.h"
#include <string.h>
#include <stdio.h>

ScriptManager::ScriptManager() : lastUpdateTime(0) {
    // Initialize all script slots
    for (int i = 0; i < MAX_SCRIPTS; i++) {
        memset(&scripts[i], 0, sizeof(ScriptInfo));
        scripts[i].state = ScriptState::EMPTY;
        strcpy(scripts[i].name, "empty");
        strcpy(scripts[i].output, "");
    }
}

void ScriptManager::begin() {
    lastUpdateTime = millis();
    
    // Initialize any audio/synthesis subsystems here
    // In a full implementation, this would:
    // - Initialize SuperCollider server
    // - Set up audio routing
    // - Configure MIDI
    
    Serial.println("ScriptManager: Initialized");
}

void ScriptManager::update() {
    unsigned long currentTime = millis();
    
    // Update at configured interval
    if (currentTime - lastUpdateTime < SCRIPT_UPDATE_INTERVAL) {
        return;
    }
    lastUpdateTime = currentTime;
    
    // Update each running script
    for (int i = 0; i < MAX_SCRIPTS; i++) {
        if (scripts[i].state == ScriptState::RUNNING) {
            executeScriptFrame(i);
        }
    }
}

bool ScriptManager::loadScript(uint8_t slot, const char* path) {
    if (slot >= MAX_SCRIPTS) {
        Serial.println("ScriptManager: Invalid slot");
        return false;
    }
    
    // Unload any existing script
    if (scripts[slot].state != ScriptState::EMPTY) {
        unloadScript(slot);
    }
    
    scripts[slot].state = ScriptState::LOADING;
    
    // Copy path
    strncpy(scripts[slot].path, path, sizeof(scripts[slot].path) - 1);
    
    // Parse script header for metadata
    if (!parseScriptHeader(slot, path)) {
        scripts[slot].state = ScriptState::ERROR;
        strcpy(scripts[slot].output, "Error: Failed to load");
        return false;
    }
    
    // Start the script automatically
    scripts[slot].state = ScriptState::RUNNING;
    scripts[slot].lastUpdate = millis();
    
    Serial.print("ScriptManager: Loaded script in slot ");
    Serial.println(slot);
    
    return true;
}

bool ScriptManager::unloadScript(uint8_t slot) {
    if (slot >= MAX_SCRIPTS) return false;
    
    // Stop script if running
    if (scripts[slot].state == ScriptState::RUNNING) {
        stopScript(slot);
    }
    
    // Clear script data
    memset(&scripts[slot], 0, sizeof(ScriptInfo));
    scripts[slot].state = ScriptState::EMPTY;
    strcpy(scripts[slot].name, "empty");
    
    Serial.print("ScriptManager: Unloaded slot ");
    Serial.println(slot);
    
    return true;
}

bool ScriptManager::startScript(uint8_t slot) {
    if (slot >= MAX_SCRIPTS) return false;
    if (scripts[slot].state == ScriptState::EMPTY) return false;
    
    scripts[slot].state = ScriptState::RUNNING;
    scripts[slot].lastUpdate = millis();
    
    return true;
}

bool ScriptManager::stopScript(uint8_t slot) {
    if (slot >= MAX_SCRIPTS) return false;
    
    if (scripts[slot].state == ScriptState::RUNNING || 
        scripts[slot].state == ScriptState::PAUSED) {
        scripts[slot].state = ScriptState::LOADING;  // Ready to run again
        strcpy(scripts[slot].output, "Stopped");
    }
    
    return true;
}

bool ScriptManager::pauseScript(uint8_t slot) {
    if (slot >= MAX_SCRIPTS) return false;
    
    if (scripts[slot].state == ScriptState::RUNNING) {
        scripts[slot].state = ScriptState::PAUSED;
        return true;
    }
    
    return false;
}

bool ScriptManager::resumeScript(uint8_t slot) {
    if (slot >= MAX_SCRIPTS) return false;
    
    if (scripts[slot].state == ScriptState::PAUSED) {
        scripts[slot].state = ScriptState::RUNNING;
        return true;
    }
    
    return false;
}

bool ScriptManager::isScriptRunning(uint8_t slot) {
    if (slot >= MAX_SCRIPTS) return false;
    return scripts[slot].state == ScriptState::RUNNING;
}

bool ScriptManager::isScriptLoaded(uint8_t slot) {
    if (slot >= MAX_SCRIPTS) return false;
    return scripts[slot].state != ScriptState::EMPTY;
}

ScriptState ScriptManager::getScriptState(uint8_t slot) {
    if (slot >= MAX_SCRIPTS) return ScriptState::EMPTY;
    return scripts[slot].state;
}

const char* ScriptManager::getScriptName(uint8_t slot) {
    if (slot >= MAX_SCRIPTS) return "";
    return scripts[slot].name;
}

const char* ScriptManager::getScriptOutput(uint8_t slot) {
    if (slot >= MAX_SCRIPTS) return "";
    return scripts[slot].output;
}

void ScriptManager::sendToScript(uint8_t slot, const char* message) {
    if (slot >= MAX_SCRIPTS) return;
    if (scripts[slot].state != ScriptState::RUNNING) return;
    
    // In a full implementation, this would send the message
    // to the script's message handler
    Serial.print("ScriptManager: Message to slot ");
    Serial.print(slot);
    Serial.print(": ");
    Serial.println(message);
}

uint8_t ScriptManager::listScripts(char scripts_list[][64], uint8_t maxScripts) {
    // Stub implementation - would scan SD card for .lua or .scd files
    // Returns some demo scripts for now
    
    const char* demoScripts[] = {
        "/scripts/awake.lua",
        "/scripts/molly_the_poly.lua",
        "/scripts/cheat_codes.lua",
        "/scripts/mx.samples.lua"
    };
    
    uint8_t count = 0;
    for (uint8_t i = 0; i < 4 && i < maxScripts; i++) {
        strncpy(scripts_list[i], demoScripts[i], 63);
        scripts_list[i][63] = '\0';
        count++;
    }
    
    return count;
}

bool ScriptManager::parseScriptHeader(uint8_t slot, const char* path) {
    // Stub implementation - would parse script file for metadata
    // For now, extract name from path
    
    const char* lastSlash = strrchr(path, '/');
    const char* filename = lastSlash ? lastSlash + 1 : path;
    
    // Copy filename as script name (without extension)
    strncpy(scripts[slot].name, filename, sizeof(scripts[slot].name) - 1);
    
    // Remove .lua or .scd extension if present
    char* dot = strrchr(scripts[slot].name, '.');
    if (dot) *dot = '\0';
    
    strcpy(scripts[slot].author, "unknown");
    strcpy(scripts[slot].version, "1.0");
    
    return true;
}

void ScriptManager::executeScriptFrame(uint8_t slot) {
    // Stub implementation - would execute one frame of the script
    // In a full implementation, this would:
    // - Run Lua/SuperCollider script cycle
    // - Update audio output
    // - Handle MIDI
    // - Update display output
    
    unsigned long runtime = millis() - scripts[slot].lastUpdate;
    
    // Update output buffer with runtime info
    snprintf(scripts[slot].output, sizeof(scripts[slot].output),
             "Running: %lu.%lus", runtime / 1000, (runtime % 1000) / 100);
}
