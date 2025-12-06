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
    {"Poliquencer", "Metropolix-style Artistic Sequencer", 2},
    {"Symphony Chord Sequencer", "4-Chord Harmonic Sequencer (Oxi-style)", 5},
    {"LFO", "Low Frequency Oscillator", 0},
    {"Envelope", "ADSR Envelope (Coming Soon)", 3},
    {"Clock", "Clock Divider (Coming Soon)", 4}
};

const uint8_t ScriptManager::scriptLibraryCount = sizeof(ScriptManager::scriptLibrary) / sizeof(ScriptLibraryEntry);

ScriptManager::ScriptManager() 
    : dacInitialized(false)
    , lastUpdateTime(0) {
    // Initialize all script slots
    for (int i = 0; i < MAX_SCRIPTS; i++) {
        memset(&scripts[i], 0, sizeof(ScriptInfo));
        scripts[i].state = ScriptState::EMPTY;
        strcpy(scripts[i].name, "empty");
        strcpy(scripts[i].output, "");
        lfoInstances[i] = nullptr;
        poliquencerInstances[i] = nullptr;
        chordSequencerInstances[i] = nullptr;
        touchTestInstances[i] = nullptr;
    }
}

void ScriptManager::begin() {
    lastUpdateTime = millis();
    
    Serial.println("ScriptManager: Initializing...");
    
    // Initialize I2C
    Wire.begin();
    
    // Initialize MCP4725 DACs
    Serial.println("ScriptManager: Initializing MCP4725 DACs...");
    bool dac1Ready = dac1.begin(MCP4725_ADDR_1);
    bool dac2Ready = dac2.begin(MCP4725_ADDR_2);
    
    if (dac1Ready && dac2Ready) {
        dacInitialized = true;
        Serial.println("  DAC1 (0x60): OK");
        Serial.println("  DAC2 (0x61): OK");
    } else {
        Serial.println("  Warning: One or more DACs not detected");
        if (!dac1Ready) Serial.println("  DAC1 (0x60): NOT FOUND");
        if (!dac2Ready) Serial.println("  DAC2 (0x61): NOT FOUND");
        dacInitialized = false;
    }
    
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
    // Basic sequencer (type 1) is deprecated
    
    // Clean up poliquencer instance if exists
    if (poliquencerInstances[slot] != nullptr) {
        poliquencerInstances[slot]->stop();
        delete poliquencerInstances[slot];
        poliquencerInstances[slot] = nullptr;
    }
    
    // Clean up chord sequencer instance if exists
    if (chordSequencerInstances[slot] != nullptr) {
        chordSequencerInstances[slot]->stop();
        delete chordSequencerInstances[slot];
        chordSequencerInstances[slot] = nullptr;
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
    
    // Check for special Input Test index (100)
    if (libraryIndex == 100) {
        if (slot >= MAX_SCRIPTS) return false;
        
        // Unload any existing script
        if (scripts[slot].state != ScriptState::EMPTY) {
            Serial.println("Unloading existing script");
            unloadScript(slot);
        }
        
        Serial.println("Creating touch test instance...");
        touchTestInstances[slot] = new TouchTestScript();
        touchTestInstances[slot]->start();
        
        strcpy(scripts[slot].name, "Input Test");
        strcpy(scripts[slot].path, "builtin://inputtest");
        scripts[slot].state = ScriptState::RUNNING;
        
        Serial.print("Loaded Input Test in slot ");
        Serial.println(slot);
        return true;
    }
    
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
        
        // Set shared DACs (LFO uses DAC1)
        if (dacInitialized) {
            lfoInstances[slot]->setDAC(&dac1, &dac2);
        }
        
        strcpy(scripts[slot].name, entry->name);
        strcpy(scripts[slot].path, "builtin://lfo");
        scripts[slot].state = ScriptState::RUNNING;
        
        Serial.print("Loaded LFO in slot ");
        Serial.println(slot);
        return true;
    } else if (entry->scriptType == 1) {  // Basic Sequencer (deprecated, type 2 is used instead)
        Serial.println("ERROR: Basic sequencer type 1 is no longer supported");
        return false;
        
        Serial.print("Loaded Sequencer in slot ");
        Serial.println(slot);
        Serial.print("Script name: ");
        Serial.println(scripts[slot].name);
        Serial.print("Script state: RUNNING\n");
        return true;
    } else if (entry->scriptType == 2) {  // Poliquencer
        Serial.println("Creating poliquencer instance...");
        poliquencerInstances[slot] = new PoliquencerScript();
        if (!poliquencerInstances[slot]->begin()) {
            Serial.println("ERROR: Poliquencer begin() failed");
            delete poliquencerInstances[slot];
            poliquencerInstances[slot] = nullptr;
            return false;
        }
        Serial.println("Poliquencer begin() successful");
        
        // Set shared DACs (Poliquencer uses DAC1 for CV, DAC2 for gate)
        if (dacInitialized) {
            poliquencerInstances[slot]->setDAC(&dac1, &dac2);
        }
        
        // Set default tempo
        poliquencerInstances[slot]->setGlobalTempo(DEFAULT_CLOCK_BPM);
        Serial.print("Tempo set to ");
        Serial.println(DEFAULT_CLOCK_BPM);
        
        strcpy(scripts[slot].name, entry->name);
        strcpy(scripts[slot].path, "builtin://poliquencer");
        scripts[slot].state = ScriptState::RUNNING;
        
        Serial.print("Loaded Poliquencer in slot ");
        Serial.println(slot);
        return true;
    } else if (entry->scriptType == 5) {  // ChordSequencer
        Serial.println("Creating chord sequencer instance...");
        chordSequencerInstances[slot] = new ChordSequencerScript();
        if (!chordSequencerInstances[slot]->begin()) {
            Serial.println("ERROR: ChordSequencer begin() failed");
            delete chordSequencerInstances[slot];
            chordSequencerInstances[slot] = nullptr;
            return false;
        }
        Serial.println("ChordSequencer begin() successful");
        
        // Set default tempo
        chordSequencerInstances[slot]->setGlobalTempo(DEFAULT_CLOCK_BPM);
        Serial.print("Tempo set to ");
        Serial.println(DEFAULT_CLOCK_BPM);
        
        strcpy(scripts[slot].name, entry->name);
        strcpy(scripts[slot].path, "builtin://chordseq");
        scripts[slot].state = ScriptState::RUNNING;
        
        Serial.print("Loaded ChordSequencer in slot ");
        Serial.println(slot);
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
    // Basic sequencer (type 1) is deprecated
    
    // Update poliquencer if this slot has one
    if (poliquencerInstances[slot] != nullptr) {
        poliquencerInstances[slot]->update();
        
        // Update display output
        poliquencerInstances[slot]->getDisplayText(scripts[slot].output, sizeof(scripts[slot].output));
        return;
    }
    
    // Update chord sequencer if this slot has one
    if (chordSequencerInstances[slot] != nullptr) {
        chordSequencerInstances[slot]->update();
        
        // Update display output
        chordSequencerInstances[slot]->getDisplayText(scripts[slot].output, sizeof(scripts[slot].output));
        return;
    }
    
    // Update touch test if this slot has one
    if (touchTestInstances[slot] != nullptr) {
        touchTestInstances[slot]->update();
        strcpy(scripts[slot].output, "Testing inputs...");
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

void ScriptManager::setLFOWaveform(uint8_t slot, uint8_t waveType) {
    if (slot >= MAX_SCRIPTS || lfoInstances[slot] == nullptr) {
        return;
    }
    lfoInstances[slot]->setWaveform(waveType);
}

void ScriptManager::setLFOFrequency(uint8_t slot, float frequency) {
    if (slot >= MAX_SCRIPTS || lfoInstances[slot] == nullptr) {
        return;
    }
    lfoInstances[slot]->setFrequency(frequency);
}

void ScriptManager::setLFOLevel(uint8_t slot, float level) {
    if (slot >= MAX_SCRIPTS || lfoInstances[slot] == nullptr) {
        return;
    }
    lfoInstances[slot]->setLevel(level);
}

bool ScriptManager::getSequencerData(uint8_t slot, uint8_t* currentStep, int8_t stepValues[8], uint8_t stepDurations[8]) {
    if (slot >= MAX_SCRIPTS) {
        Serial.print("getSequencerData: Invalid slot ");
        Serial.println(slot);
        return false;
    }
    // Basic sequencer (type 1) is deprecated
    return false;
}

void ScriptManager::setGlobalTempo(float bpm) {
    // Update tempo for all running sequencers (poliquencer only, basic seq deprecated)
    for (int i = 0; i < MAX_SCRIPTS; i++) {
        if (poliquencerInstances[i] != nullptr) {
            poliquencerInstances[i]->setGlobalTempo(bpm);
        }
        if (chordSequencerInstances[i] != nullptr) {
            chordSequencerInstances[i]->setGlobalTempo(bpm);
        }
    }
}

void ScriptManager::setSequencerStepValue(uint8_t slot, uint8_t step, int8_t value) {
    // Basic sequencer (type 1) is deprecated
}

void ScriptManager::setSequencerStepDuration(uint8_t slot, uint8_t step, uint8_t duration) {
    // Basic sequencer (type 1) is deprecated
}

// ========== POLIQUENCER METHODS ==========

bool ScriptManager::getPoliquencerData(uint8_t slot, uint8_t* currentStep, uint8_t* currentBeat, int8_t stepValues[8], uint8_t stepDurations[8], uint8_t gateModes[8], uint8_t* direction, bool* steamTrigger) {
    if (slot >= MAX_SCRIPTS || poliquencerInstances[slot] == nullptr) {
        return false;
    }
    
    if (currentStep) *currentStep = poliquencerInstances[slot]->getCurrentStep();
    if (currentBeat) *currentBeat = poliquencerInstances[slot]->getCurrentBeat();
    if (direction) *direction = (uint8_t)poliquencerInstances[slot]->getDirection();
    if (steamTrigger) *steamTrigger = poliquencerInstances[slot]->isSteamTrigger();
    
    if (stepValues) {
        for (int i = 0; i < 8; i++) {
            stepValues[i] = poliquencerInstances[slot]->getStepValue(i);
        }
    }
    if (stepDurations) {
        for (int i = 0; i < 8; i++) {
            stepDurations[i] = poliquencerInstances[slot]->getStepDuration(i);
        }
    }
    if (gateModes) {
        for (int i = 0; i < 8; i++) {
            gateModes[i] = (uint8_t)poliquencerInstances[slot]->getStepGateMode(i);
        }
    }
    
    return true;
}

void ScriptManager::setPoliquencerStepValue(uint8_t slot, uint8_t step, int8_t value) {
    if (slot >= MAX_SCRIPTS || poliquencerInstances[slot] == nullptr) return;
    poliquencerInstances[slot]->setStepValue(step, value);
}

void ScriptManager::setPoliquencerStepDuration(uint8_t slot, uint8_t step, uint8_t duration) {
    if (slot >= MAX_SCRIPTS || poliquencerInstances[slot] == nullptr) return;
    poliquencerInstances[slot]->setStepDuration(step, duration);
}

void ScriptManager::setPoliquencerStepGateMode(uint8_t slot, uint8_t step, uint8_t gateMode) {
    if (slot >= MAX_SCRIPTS || poliquencerInstances[slot] == nullptr) return;
    poliquencerInstances[slot]->setStepGateMode(step, (GateMode)gateMode);
}

void ScriptManager::setPoliquencerDirection(uint8_t slot, uint8_t direction) {
    if (slot >= MAX_SCRIPTS || poliquencerInstances[slot] == nullptr) return;
    poliquencerInstances[slot]->setDirection((DirectionMode)direction);
}

// ========== CHORD SEQUENCER METHODS ==========

bool ScriptManager::getChordSequencerData(uint8_t slot, uint8_t chordRoots[4], uint8_t chordTypes[4], uint8_t chordBeats[4], uint8_t* currentChordSlot, uint8_t* beatCounter) {
    if (slot >= MAX_SCRIPTS || chordSequencerInstances[slot] == nullptr) {
        return false;
    }
    
    if (currentChordSlot) *currentChordSlot = chordSequencerInstances[slot]->getCurrentChordSlot();
    if (beatCounter) *beatCounter = chordSequencerInstances[slot]->getBeatCounter();
    
    if (chordRoots && chordTypes && chordBeats) {
        for (int i = 0; i < 4; i++) {
            uint8_t root;
            ChordType type;
            chordSequencerInstances[slot]->getChord(i, &root, &type);
            chordRoots[i] = root;
            chordTypes[i] = (uint8_t)type;
            chordBeats[i] = chordSequencerInstances[slot]->getChordBeats(i);
        }
    }
    
    return true;
}

void ScriptManager::setChordSequencerChord(uint8_t slot, uint8_t chordSlot, uint8_t rootNote, uint8_t chordType) {
    if (slot >= MAX_SCRIPTS || chordSlot >= 4 || chordSequencerInstances[slot] == nullptr) {
        return;
    }
    
    ChordType type = (chordType == 0) ? CHORD_MAJOR : CHORD_MINOR;
    chordSequencerInstances[slot]->setChord(chordSlot, rootNote, type);
}

