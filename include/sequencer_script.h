/**
 * Sequencer Script Header
 * 
 * 8-step musical sequencer with CV and Gate output
 * Supports major/minor scales with V/octave standard
 */

#ifndef SEQUENCER_SCRIPT_H
#define SEQUENCER_SCRIPT_H

#include <Arduino.h>
#include <Adafruit_MCP4725.h>
#include "config.h"

class SequencerScript {
public:
    SequencerScript();
    
    // Initialize the sequencer
    bool begin();
    
    // Update sequencer state (call frequently)
    void update();
    
    // Stop the sequencer
    void stop();
    
    // Sequencer parameters
    void setStepValue(uint8_t step, int8_t semitones); // -12 to +12 (2 octaves)
    void setStepDuration(uint8_t step, uint8_t beats); // 1-8 beats per step
    void setRootNote(uint8_t note);     // 0-11 (C to B)
    void setScale(uint8_t scale);       // 0=Major, 1=Minor
    void setCurrentStep(uint8_t step);  // Manually set current step (0-7)
    void setGlobalTempo(float bpm);     // Set tempo from global clock
    
    // Getters
    int8_t getStepValue(uint8_t step) const { return (step < 8) ? stepValues[step] : 0; }
    uint8_t getStepDuration(uint8_t step) const { return (step < 8) ? stepDurations[step] : 1; }
    uint8_t getCurrentStep() const { return currentStep; }
    uint8_t getRootNote() const { return rootNote; }
    uint8_t getScale() const { return scaleType; }
    bool isGateHigh() const { return gateHigh; }
    
    // Get display text
    void getDisplayText(char* buffer, size_t bufferSize);
    
private:
    // DAC objects
    Adafruit_MCP4725 dacCV;
    Adafruit_MCP4725 dacGate;
    bool dacCVInitialized;
    bool dacGateInitialized;
    
    // Sequencer state
    int8_t stepValues[8];      // Step values in scale degrees
    uint8_t stepDurations[8];  // Duration in beats (1-8) for each step
    uint8_t currentStep;       // Current step (0-7)
    uint8_t beatCounter;       // Counts beats within current step
    uint8_t rootNote;          // Root note (0-11: C to B)
    uint8_t scaleType;         // 0=Major, 1=Minor
    bool gateHigh;             // Current gate state
    
    // Timing
    unsigned long lastStepMicros;
    unsigned long stepDurationMicros;
    unsigned long gateOnMicros;
    const unsigned long GATE_LENGTH_PERCENT = 50; // Gate on for 50% of step
    
    // Scale tables (in semitones from root)
    static const int8_t majorScale[7];
    static const int8_t minorScale[7];
    
    // Helper functions
    void updateStepTiming();
    float calculateCVVoltage(int8_t stepValue);
    uint16_t voltageToDACValue(float volts);
    void outputCV(float volts);
    void outputGate(bool high);
};

#endif // SEQUENCER_SCRIPT_H
