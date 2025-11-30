/**
 * Script Manager
 * 
 * Manages up to 4 concurrent SuperCollider-compatible scripts
 * Similar to Norns Shield scripting environment
 */

#ifndef SCRIPT_MANAGER_H
#define SCRIPT_MANAGER_H

#include <Arduino.h>
#include "config.h"

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
    
    // List available scripts (would scan SD card in real implementation)
    uint8_t listScripts(char scripts[][64], uint8_t maxScripts);

private:
    ScriptInfo scripts[MAX_SCRIPTS];
    
    // Timing for script updates
    unsigned long lastUpdateTime;
    
    // Helper methods
    bool parseScriptHeader(uint8_t slot, const char* path);
    void executeScriptFrame(uint8_t slot);
};

#endif // SCRIPT_MANAGER_H
