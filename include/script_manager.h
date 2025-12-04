/**
 * Script Manager
 * 
 * Manages up to 4 concurrent SuperCollider-compatible scripts
 * Similar to Norns Shield scripting environment
 */

#ifndef SCRIPT_MANAGER_H
#define SCRIPT_MANAGER_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_MCP4725.h>
#include "config.h"
#include "lfo_script.h"
#include "poliquencer_script.h"
#include "chord_sequencer_script.h"
#include "touch_test_script.h"

// Script library entry
struct ScriptLibraryEntry {
    char name[32];
    char description[64];
    uint8_t scriptType;  // 0=LFO, 1=Sequencer, etc.
};

// Script state
enum class ScriptState {
    EMPTY,
    LOADING,
    RUNNING,
    PAUSED,
    ERROR
};

// Script information structure
struct ScriptInfo {
    char name[32];
    char path[64];
    char author[32];
    char version[16];
    ScriptState state;
    unsigned long lastUpdate;
    char output[128];  // Buffer for script output/display
};

class ScriptManager {
public:
    ScriptManager();
    
    void begin();
    void update();
    
    // Script lifecycle
    bool loadScript(uint8_t slot, const char* path);
    bool unloadScript(uint8_t slot);
    bool startScript(uint8_t slot);
    bool stopScript(uint8_t slot);
    bool pauseScript(uint8_t slot);
    bool resumeScript(uint8_t slot);
    
    // Script queries
    bool isScriptRunning(uint8_t slot);
    bool isScriptLoaded(uint8_t slot);
    ScriptState getScriptState(uint8_t slot);
    const char* getScriptName(uint8_t slot);
    const char* getScriptOutput(uint8_t slot);
    
    // Script communication
    void sendToScript(uint8_t slot, const char* message);
    
    // Script library
    uint8_t getScriptLibraryCount();
    const ScriptLibraryEntry* getScriptLibraryEntry(uint8_t index);
    bool loadScriptFromLibrary(uint8_t slot, uint8_t libraryIndex);
    
    // LFO access
    bool getLFOWaveformData(uint8_t slot, uint8_t* waveType, float* phase);
    void setLFOWaveform(uint8_t slot, uint8_t waveType);
    void setLFOFrequency(uint8_t slot, float frequency);
    void setLFOLevel(uint8_t slot, float level);
    
    // Sequencer access
    bool getSequencerData(uint8_t slot, uint8_t* currentStep, int8_t stepValues[8], uint8_t stepDurations[8]);
    void setGlobalTempo(float bpm);  // Set tempo for all sequencers
    void setSequencerStepValue(uint8_t slot, uint8_t step, int8_t value);
    void setSequencerStepDuration(uint8_t slot, uint8_t step, uint8_t duration);
    
    // Poliquencer access
    bool getPoliquencerData(uint8_t slot, uint8_t* currentStep, uint8_t* currentBeat, int8_t stepValues[8], uint8_t stepDurations[8], uint8_t gateModes[8], uint8_t* direction, bool* steamTrigger);
    void setPoliquencerStepValue(uint8_t slot, uint8_t step, int8_t value);
    
    // ChordSequencer access
    bool getChordSequencerData(uint8_t slot, uint8_t chordRoots[4], uint8_t chordTypes[4], uint8_t chordBeats[4], uint8_t* currentChordSlot, uint8_t* beatCounter);
    void setPoliquencerStepDuration(uint8_t slot, uint8_t step, uint8_t duration);
    void setPoliquencerStepGateMode(uint8_t slot, uint8_t step, uint8_t gateMode);
    void setPoliquencerDirection(uint8_t slot, uint8_t direction);
    
    // DAC access (for testing)
    Adafruit_MCP4725* getDAC1() { return &dac1; }
    Adafruit_MCP4725* getDAC2() { return &dac2; }

private:
    ScriptInfo scripts[MAX_SCRIPTS];
    LFOScript* lfoInstances[MAX_SCRIPTS];  // LFO instance per slot
    PoliquencerScript* poliquencerInstances[MAX_SCRIPTS];  // Poliquencer instance per slot
    ChordSequencerScript* chordSequencerInstances[MAX_SCRIPTS];  // Chord sequencer instance per slot
    TouchTestScript* touchTestInstances[MAX_SCRIPTS];  // Touch test instance per slot
    
    // Shared DAC instances (2x MCP4725)
    Adafruit_MCP4725 dac1;  // CV output (address 0x60)
    Adafruit_MCP4725 dac2;  // Gate/CV output (address 0x61)
    bool dacInitialized;
    
    // Script library
    static const ScriptLibraryEntry scriptLibrary[];
    static const uint8_t scriptLibraryCount;
    
    // Timing for script updates
    unsigned long lastUpdateTime;
    
    // Helper methods
    bool parseScriptHeader(uint8_t slot, const char* path);
    void executeScriptFrame(uint8_t slot);
};

#endif // SCRIPT_MANAGER_H
