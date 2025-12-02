/**
 * Sequencer Script Implementation
 * 
 * 8-step musical sequencer with quantized scales
 * Eurorack V/octave standard: 1V/octave, C4 = 2V
 */

#include "sequencer_script.h"
#include <Wire.h>

// Scale definitions (semitones from root)
// Major: W-W-H-W-W-W-H (Ionian)
const int8_t SequencerScript::majorScale[7] = {0, 2, 4, 5, 7, 9, 11};
// Natural Minor: W-H-W-W-H-W-W (Aeolian)
const int8_t SequencerScript::minorScale[7] = {0, 2, 3, 5, 7, 8, 10};

SequencerScript::SequencerScript()
    : dacCVInitialized(false)
    , dacGateInitialized(false)
    , currentStep(0)
    , beatCounter(0)
    , rootNote(0)  // C
    , scaleType(0) // Major
    , gateHigh(false)
    , lastStepMicros(0)
    , stepDurationMicros(500000)  // 120 BPM default
    , gateOnMicros(0) {
    
    // Initialize steps to ascending scale pattern
    for (int i = 0; i < 8; i++) {
        stepValues[i] = i;  // 0-7 scale degrees
        stepDurations[i] = 1;  // 1 beat per step by default
    }
}

bool SequencerScript::begin() {
    // Initialize I2C if not already done
    Wire.begin();
    
    // Initialize CV DAC
    if (!dacCV.begin(MCP4725_CV_ADDR)) {
        Serial.println("SEQ: WARNING - CV DAC (0x60) not found");
        dacCVInitialized = false;
    } else {
        dacCVInitialized = true;
        dacCV.setVoltage(0, false);
        Serial.println("SEQ: CV DAC initialized");
    }
    
    // Initialize Gate DAC
    if (!dacGate.begin(MCP4725_GATE_ADDR)) {
        Serial.println("SEQ: WARNING - Gate DAC (0x61) not found");
        Serial.println("     Check A0 pin is tied to VDD for address 0x61");
        dacGateInitialized = false;
    } else {
        dacGateInitialized = true;
        dacGate.setVoltage(0, false);
        Serial.println("SEQ: Gate DAC initialized");
    }
    
    lastStepMicros = micros();
    
    return true; // Always return true for visual operation
}

void SequencerScript::update() {
    unsigned long currentMicros = micros();
    unsigned long elapsed = currentMicros - lastStepMicros;
    
    // Check if it's time for next beat
    if (elapsed >= stepDurationMicros) {
        beatCounter++;
        
        // Check if we've completed all beats for this step
        if (beatCounter >= stepDurations[currentStep]) {
            // Advance to next step
            currentStep = (currentStep + 1) % 8;
            beatCounter = 0;
        }
        
        lastStepMicros = currentMicros;
        gateOnMicros = currentMicros;
        
        // Output CV for this step (doesn't change during beat repeats)
        float cv = calculateCVVoltage(stepValues[currentStep]);
        outputCV(cv);
        
        // Turn gate high for each beat
        gateHigh = true;
        outputGate(true);
    }
    
    // Check if gate should turn off (50% duty cycle)
    if (gateHigh && (currentMicros - gateOnMicros) >= (stepDurationMicros * GATE_LENGTH_PERCENT / 100)) {
        gateHigh = false;
        outputGate(false);
    }
}

void SequencerScript::stop() {
    // Turn off outputs
    outputCV(0.0f);
    outputGate(false);
    gateHigh = false;
}

void SequencerScript::setGlobalTempo(float bpm) {
    if (bpm < 20.0f) bpm = 20.0f;
    if (bpm > 300.0f) bpm = 300.0f;
    // Calculate step duration in microseconds
    stepDurationMicros = (unsigned long)((60.0f / bpm) * 1000000.0f);
}

void SequencerScript::setStepValue(uint8_t step, int8_t semitones) {
    if (step >= 8) return;
    
    // Clamp to 2 octaves range (-12 to +12 semitones)
    if (semitones < -12) semitones = -12;
    if (semitones > 12) semitones = 12;
    
    stepValues[step] = semitones;
}

void SequencerScript::setStepDuration(uint8_t step, uint8_t beats) {
    if (step >= 8) return;
    
    // Clamp to 1-8 beats
    if (beats < 1) beats = 1;
    if (beats > 8) beats = 8;
    
    stepDurations[step] = beats;
}

void SequencerScript::setRootNote(uint8_t note) {
    if (note < 12) {
        rootNote = note;
    }
}

void SequencerScript::setScale(uint8_t scale) {
    if (scale <= 1) {
        scaleType = scale;
    }
}

void SequencerScript::setCurrentStep(uint8_t step) {
    if (step < 8) {
        currentStep = step;
    }
}

float SequencerScript::calculateCVVoltage(int8_t stepValue) {
    // Eurorack V/octave standard:
    // - 1V per octave
    // - C4 (MIDI 60) = 2V
    // - Each semitone = 1/12 V = 0.08333V
    
    // Get scale and convert scale degree to semitones
    const int8_t* scale = (scaleType == 0) ? majorScale : minorScale;
    
    // Calculate octave offset and scale degree
    int8_t octaveOffset = stepValue / 7;  // Which octave (can be negative)
    int8_t scaleDegree = stepValue % 7;
    if (stepValue < 0 && scaleDegree != 0) {
        octaveOffset--;
        scaleDegree = 7 + scaleDegree;
    }
    
    // Get semitones from scale
    int8_t semitones = scale[scaleDegree] + (octaveOffset * 12);
    
    // Add root note offset
    semitones += rootNote;
    
    // Calculate voltage: C4 (MIDI 60) = 2V
    // Each semitone = 1/12 V
    // Start from C4 reference
    float voltage = 2.0f + (semitones / 12.0f);
    
    // Clamp to 0-5V range
    if (voltage < 0.0f) voltage = 0.0f;
    if (voltage > 5.0f) voltage = 5.0f;
    
    return voltage;
}

uint16_t SequencerScript::voltageToDACValue(float volts) {
    if (volts < 0.0f) volts = 0.0f;
    if (volts > DAC_MAX_VOLTAGE) volts = DAC_MAX_VOLTAGE;
    
    uint16_t dacValue = (uint16_t)((volts / DAC_MAX_VOLTAGE) * DAC_MAX_VALUE);
    if (dacValue > DAC_MAX_VALUE) dacValue = DAC_MAX_VALUE;
    
    return dacValue;
}

void SequencerScript::outputCV(float volts) {
    if (dacCVInitialized) {
        uint16_t dacValue = voltageToDACValue(volts);
        dacCV.setVoltage(dacValue, false);
    }
}

void SequencerScript::outputGate(bool high) {
    if (dacGateInitialized) {
        float voltage = high ? GATE_HIGH_VOLTAGE : GATE_LOW_VOLTAGE;
        uint16_t dacValue = voltageToDACValue(voltage);
        dacGate.setVoltage(dacValue, false);
    }
}

void SequencerScript::getDisplayText(char* buffer, size_t bufferSize) {
    const char* noteNames[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
    const char* scaleNames[] = {"MAJ", "MIN"};
    
    snprintf(buffer, bufferSize,
             "SEQ\nStep: %d\nRoot: %s %s",
             currentStep + 1,
             noteNames[rootNote],
             scaleNames[scaleType]);
}
