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

// Script library definition
const ScriptLibraryEntry ScriptManager::scriptLibrary[] = {
    {"LFO", "Low Frequency Oscillator", 0},
    {"8 Step Sequencer", "Musical Sequencer with CV/Gate", 1},
    {"Envelope", "ADSR Envelope (Coming Soon)", 2},
    {"Clock", "Clock Divider (Coming Soon)", 3}
};

const uint8_t ScriptManager::scriptLibraryCount = sizeof(ScriptManager::scriptLibrary) / sizeof(ScriptLibraryEntry);

ScriptManager::ScriptManager() : lastUpdateTime(0) {
    // Initialize all script slots
    for (int i = 0; i < MAX_SCRIPTS; i++) {
        memset(&scripts[i], 0, sizeof(ScriptInfo));
        scripts[i].state = ScriptState::EMPTY;
        strcpy(scripts[i].name, "empty");
        strcpy(scripts[i].output, "");
        lfoInstances[i] = nullptr;
        sequencerInstances[i] = nullptr;
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
    // Update each running script (no throttling for LFO precision)
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
    
    // Copy path with null termination
    strncpy(scripts[slot].path, path, sizeof(scripts[slot].path) - 1);
    scripts[slot].path[sizeof(scripts[slot].path) - 1] = '\0';
    
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
    
    // Clean up LFO instance if exists
    if (lfoInstances[slot] != nullptr) {
        lfoInstances[slot]->stop();
        delete lfoInstances[slot];
        lfoInstances[slot] = nullptr;
    }
    
    // Clean up sequencer instance if exists
    if (sequencerInstances[slot] != nullptr) {
        sequencerInstances[slot]->stop();
        delete sequencerInstances[slot];
        sequencerInstances[slot] = nullptr;
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

uint8_t ScriptManager::getScriptLibraryCount() {
    return scriptLibraryCount;
}

const ScriptLibraryEntry* ScriptManager::getScriptLibraryEntry(uint8_t index) {
    if (index >= scriptLibraryCount) return nullptr;
    return &scriptLibrary[index];
}

bool ScriptManager::loadScriptFromLibrary(uint8_t slot, uint8_t libraryIndex) {
    Serial.print("loadScriptFromLibrary called: slot=");
    Serial.print(slot);
    Serial.print(", libraryIndex=");
    Serial.println(libraryIndex);
    
    if (slot >= MAX_SCRIPTS || libraryIndex >= scriptLibraryCount) {
        Serial.println("ERROR: Invalid slot or library index");
        Serial.print("  MAX_SCRIPTS=");
        Serial.print(MAX_SCRIPTS);
        Serial.print(", scriptLibraryCount=");
        Serial.println(scriptLibraryCount);
        return false;
    }
    
    // Unload any existing script
    if (scripts[slot].state != ScriptState::EMPTY) {
        Serial.println("Unloading existing script");
        unloadScript(slot);
    }
    
    const ScriptLibraryEntry* entry = &scriptLibrary[libraryIndex];
    Serial.print("Entry name: ");
    Serial.println(entry->name);
    Serial.print("Entry scriptType: ");
    Serial.println(entry->scriptType);
    
    // Load based on script type
    if (entry->scriptType == 0) {  // LFO
        lfoInstances[slot] = new LFOScript();
        if (!lfoInstances[slot]->begin()) {
            delete lfoInstances[slot];
            lfoInstances[slot] = nullptr;
            return false;
        }
        
        strcpy(scripts[slot].name, entry->name);
        strcpy(scripts[slot].path, "builtin://lfo");
        scripts[slot].state = ScriptState::RUNNING;
        
        Serial.print("Loaded LFO in slot ");
        Serial.println(slot);
        return true;
    } else if (entry->scriptType == 1) {  // Sequencer
        Serial.println("Creating sequencer instance...");
        sequencerInstances[slot] = new SequencerScript();
        if (!sequencerInstances[slot]->begin()) {
            Serial.println("ERROR: Sequencer begin() failed");
            delete sequencerInstances[slot];
            sequencerInstances[slot] = nullptr;
            return false;
        }
        Serial.println("Sequencer begin() successful");
        
        // Set default tempo (will be updated from UI global clock)
        sequencerInstances[slot]->setGlobalTempo(DEFAULT_CLOCK_BPM);
        Serial.print("Tempo set to ");
        Serial.println(DEFAULT_CLOCK_BPM);
        
        strcpy(scripts[slot].name, entry->name);
        strcpy(scripts[slot].path, "builtin://sequencer");
        scripts[slot].state = ScriptState::RUNNING;
        
        Serial.print("Loaded Sequencer in slot ");
        Serial.println(slot);
        Serial.print("Script name: ");
        Serial.println(scripts[slot].name);
        Serial.print("Script state: RUNNING\n");
        return true;
    }
    
    // Other script types not yet implemented
    strcpy(scripts[slot].output, "Coming soon");
    return false;
}

bool ScriptManager::parseScriptHeader(uint8_t slot, const char* path) {
    // Stub implementation - would parse script file for metadata
    // For now, extract name from path
    
    const char* lastSlash = strrchr(path, '/');
    const char* filename = lastSlash ? lastSlash + 1 : path;
    
    // Copy filename as script name (without extension) with null termination
    strncpy(scripts[slot].name, filename, sizeof(scripts[slot].name) - 1);
    scripts[slot].name[sizeof(scripts[slot].name) - 1] = '\0';
    
    // Remove .lua or .scd extension if present
    char* dot = strrchr(scripts[slot].name, '.');
    if (dot) *dot = '\0';
    
    strcpy(scripts[slot].author, "unknown");
    strcpy(scripts[slot].version, "1.0");
    
    return true;
}

void ScriptManager::executeScriptFrame(uint8_t slot) {
    // Update LFO if this slot has one
    if (lfoInstances[slot] != nullptr) {
        lfoInstances[slot]->update();
        
        // Update display output
        lfoInstances[slot]->getDisplayText(scripts[slot].output, sizeof(scripts[slot].output));
        return;
    }
    
    // Update sequencer if this slot has one
    if (sequencerInstances[slot] != nullptr) {
        sequencerInstances[slot]->update();
        
        // Update display output
        sequencerInstances[slot]->getDisplayText(scripts[slot].output, sizeof(scripts[slot].output));
        return;
    }
    
    // Other script types...
    unsigned long runtime = millis() - scripts[slot].lastUpdate;
    snprintf(scripts[slot].output, sizeof(scripts[slot].output),
             "Running: %lu.%lus", runtime / 1000, (runtime % 1000) / 100);
}

bool ScriptManager::getLFOWaveformData(uint8_t slot, uint8_t* waveType, float* phase) {
    if (slot >= MAX_SCRIPTS || lfoInstances[slot] == nullptr) {
        return false;
    }
    
    if (waveType) *waveType = lfoInstances[slot]->getWaveform();
    if (phase) *phase = lfoInstances[slot]->getCurrentValue() * 2.0f * PI;
    return true;
}

bool ScriptManager::getSequencerData(uint8_t slot, uint8_t* currentStep, int8_t stepValues[8]) {
    if (slot >= MAX_SCRIPTS) {
        Serial.print("getSequencerData: Invalid slot ");
        Serial.println(slot);
        return false;
    }
    if (sequencerInstances[slot] == nullptr) {
        // This is normal when LFO is running
        return false;
    }
    
    if (currentStep) *currentStep = sequencerInstances[slot]->getCurrentStep();
    if (stepValues) {
        for (int i = 0; i < 8; i++) {
            stepValues[i] = sequencerInstances[slot]->getStepValue(i);
        }
    }
    return true;
}

void ScriptManager::setGlobalTempo(float bpm) {
    // Update tempo for all running sequencers
    for (int i = 0; i < MAX_SCRIPTS; i++) {
        if (sequencerInstances[i] != nullptr) {
            sequencerInstances[i]->setGlobalTempo(bpm);
        }
    }
}

void ScriptManager::setSequencerStepValue(uint8_t slot, uint8_t step, int8_t value) {
    if (slot >= MAX_SCRIPTS || sequencerInstances[slot] == nullptr) return;
    sequencerInstances[slot]->setStepValue(step, value);
}
