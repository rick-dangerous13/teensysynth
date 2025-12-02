/**
 * SteampunQuencer Script Header
 * 
 * Metropolix-inspired 8-step sequencer with steampunk aesthetic
 * Features: Gate modes (normal/skip/slide), direction control, ratcheting
 */

#ifndef STEAMPUNQUENCER_SCRIPT_H
#define STEAMPUNQUENCER_SCRIPT_H

#include <Arduino.h>
#include <Adafruit_MCP4725.h>
#include "config.h"

// Gate modes for each step
enum GateMode {
    GATE_NORMAL = 0,  // Full gate output
    GATE_SKIP = 1,    // No gate output
    GATE_SLIDE = 2    // Smooth CV transition (portamento)
};

// Sequencer direction modes
enum DirectionMode {
    DIR_FORWARD = 0,   // 1→2→3→...→8→1
    DIR_REVERSE = 1,   // 8→7→6→...→1→8
    DIR_PENDULUM = 2,  // 1→8→1→8 (ping-pong)
    DIR_RANDOM = 3     // Random walk
};

class SteampunquencerScript {
public:
    SteampunquencerScript();
    
    // Initialize the sequencer
    bool begin();
    
    // Update sequencer state (call frequently)
    void update();
    
    // Stop the sequencer
    void stop();
    
    // Step parameters
    void setStepValue(uint8_t step, int8_t semitones); // -12 to +12 (2 octaves)
    void setStepDuration(uint8_t step, uint8_t beats); // 1-8 beats per step (ratchets)
    void setStepGateMode(uint8_t step, GateMode mode); // Normal/Skip/Slide
    
    // Global parameters
    void setRootNote(uint8_t note);        // 0-11 (C to B)
    void setScale(uint8_t scale);          // 0=Major, 1=Minor
    void setDirection(DirectionMode dir);  // Forward/Reverse/Pendulum/Random
    void setCurrentStep(uint8_t step);     // Manually set current step (0-7)
    void setGlobalTempo(float bpm);        // Set tempo from global clock
    void setPortamentoTime(uint16_t ms);   // Slide time in milliseconds
    
    // Getters
    int8_t getStepValue(uint8_t step) const { return (step < 8) ? stepValues[step] : 0; }
    uint8_t getStepDuration(uint8_t step) const { return (step < 8) ? stepDurations[step] : 1; }
    GateMode getStepGateMode(uint8_t step) const { return (step < 8) ? stepGateModes[step] : GATE_NORMAL; }
    uint8_t getCurrentStep() const { return currentStep; }
    uint8_t getRootNote() const { return rootNote; }
    uint8_t getScale() const { return scaleType; }
    DirectionMode getDirection() const { return direction; }
    bool isGateHigh() const { return gateHigh; }
    uint8_t getCurrentBeat() const { return beatCounter; } // For ratchet animation
    bool isSteamTrigger() const { return steamTrigger; }   // For steam puff animation
    
    // Get display text
    void getDisplayText(char* buffer, size_t bufferSize);
    
private:
    // DAC objects
    Adafruit_MCP4725 dacCV;
    Adafruit_MCP4725 dacGate;
    bool dacCVInitialized;
    bool dacGateInitialized;
    
    // Sequencer state
    int8_t stepValues[8];       // Step values in semitones (-12 to +12)
    uint8_t stepDurations[8];   // Duration in beats (1-8) for each step
    GateMode stepGateModes[8];  // Gate mode for each step
    uint8_t currentStep;        // Current step (0-7)
    uint8_t beatCounter;        // Counts beats within current step
    uint8_t rootNote;           // Root note (0-11: C to B)
    uint8_t scaleType;          // 0=Major, 1=Minor
    DirectionMode direction;    // Sequencer direction
    bool gateHigh;              // Current gate state
    bool steamTrigger;          // Flag for UI animation (steam puff)
    int8_t pendulumDirection;   // 1=forward, -1=reverse (for pendulum mode)
    
    // Timing
    unsigned long lastStepMicros;
    unsigned long stepDurationMicros;
    unsigned long gateOnMicros;
    const unsigned long GATE_LENGTH_PERCENT = 50; // Gate on for 50% of step
    
    // Portamento (slide mode)
    uint16_t portamentoTimeMs;
    float currentCV;            // Current CV voltage (for smooth transitions)
    float targetCV;             // Target CV voltage
    unsigned long slideStartMicros;
    
    // Scale tables (in semitones from root)
    static const int8_t majorScale[7];
    static const int8_t minorScale[7];
    
    // Helper functions
    void advanceStep();
    uint8_t getNextStep();
    float calculateCVVoltage(int8_t stepValue);
    uint16_t voltageToDACValue(float volts);
    void outputCV(float volts);
    void outputGate(bool high);
    void updatePortamento();
};

#endif // STEAMPUNQUENCER_SCRIPT_H
