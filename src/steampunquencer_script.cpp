/**
 * SteampunQuencer Script Implementation
 * 
 * Metropolix-inspired sequencer with gate modes and direction control
 * Victorian-era industrial aesthetics meet modern eurorack sequencing
 */

#include "steampunquencer_script.h"
#include <Wire.h>

// Scale definitions (semitones from root)
const int8_t SteampunquencerScript::majorScale[7] = {0, 2, 4, 5, 7, 9, 11};
const int8_t SteampunquencerScript::minorScale[7] = {0, 2, 3, 5, 7, 8, 10};

SteampunquencerScript::SteampunquencerScript()
    : dacCVInitialized(false)
    , dacGateInitialized(false)
    , currentStep(0)
    , beatCounter(0)
    , rootNote(0)  // C
    , scaleType(0) // Major
    , direction(DIR_FORWARD)
    , gateHigh(false)
    , steamTrigger(false)
    , pendulumDirection(1)
    , lastStepMicros(0)
    , stepDurationMicros(500000)  // 120 BPM default
    , gateOnMicros(0)
    , portamentoTimeMs(50)
    , currentCV(2.0f)
    , targetCV(2.0f)
    , slideStartMicros(0) {
    
    // Initialize steps with interesting steampunk pattern
    // Pattern: Root, 3rd, 5th, octave, 7th, 5th, 4th, 2nd
    int8_t pattern[8] = {0, 4, 7, 12, 10, 7, 5, 2};
    for (int i = 0; i < 8; i++) {
        stepValues[i] = pattern[i];
        stepDurations[i] = 1;  // 1 beat per step by default
        stepGateModes[i] = GATE_NORMAL;
    }
    
    // Add some variety: make step 4 a slide, step 6 a skip
    stepGateModes[3] = GATE_SLIDE;
    stepGateModes[5] = GATE_SKIP;
}

bool SteampunquencerScript::begin() {
    // Initialize I2C if not already done
    Wire.begin();
    
    // Initialize CV DAC
    if (!dacCV.begin(MCP4725_CV_ADDR)) {
        Serial.println("SPNQ: WARNING - CV DAC (0x60) not found");
        dacCVInitialized = false;
    } else {
        dacCVInitialized = true;
        dacCV.setVoltage(0, false);
        Serial.println("SPNQ: CV DAC initialized");
    }
    
    // Initialize Gate DAC
    if (!dacGate.begin(MCP4725_GATE_ADDR)) {
        Serial.println("SPNQ: WARNING - Gate DAC (0x61) not found");
        dacGateInitialized = false;
    } else {
        dacGateInitialized = true;
        dacGate.setVoltage(0, false);
        Serial.println("SPNQ: Gate DAC initialized");
    }
    
    lastStepMicros = micros();
    
    // Calculate initial CV
    targetCV = calculateCVVoltage(stepValues[currentStep]);
    currentCV = targetCV;
    
    return true;
}

void SteampunquencerScript::update() {
    unsigned long currentMicros = micros();
    unsigned long elapsed = currentMicros - lastStepMicros;
    
    // Update portamento if sliding
    if (currentCV != targetCV) {
        updatePortamento();
    }
    
    // Check if it's time for next beat
    if (elapsed >= stepDurationMicros) {
        beatCounter++;
        steamTrigger = false; // Reset steam trigger
        
        // Check if we've completed all beats for this step
        if (beatCounter >= stepDurations[currentStep]) {
            // Advance to next step
            advanceStep();
            beatCounter = 0;
        }
        
        lastStepMicros = currentMicros;
        gateOnMicros = currentMicros;
        
        // Calculate target CV for this step
        targetCV = calculateCVVoltage(stepValues[currentStep]);
        
        // Handle gate mode
        GateMode gateMode = stepGateModes[currentStep];
        
        if (gateMode == GATE_SKIP) {
            // Skip: no gate, but CV still updates
            gateHigh = false;
            outputGate(false);
            currentCV = targetCV; // Instant CV change
            outputCV(currentCV);
        } else if (gateMode == GATE_SLIDE) {
            // Slide: portamento to new CV, gate stays high
            slideStartMicros = currentMicros;
            if (!gateHigh) {
                gateHigh = true;
                outputGate(true);
                steamTrigger = true; // Trigger steam animation
            }
            // CV will be updated by updatePortamento()
        } else {
            // Normal: instant CV change, gate pulse
            currentCV = targetCV;
            outputCV(currentCV);
            gateHigh = true;
            outputGate(true);
            steamTrigger = true; // Trigger steam animation
        }
    }
    
    // Check if gate should turn off (not in slide mode)
    if (gateHigh && stepGateModes[currentStep] != GATE_SLIDE) {
        if ((currentMicros - gateOnMicros) >= (stepDurationMicros * GATE_LENGTH_PERCENT / 100)) {
            gateHigh = false;
            outputGate(false);
        }
    }
}

void SteampunquencerScript::advanceStep() {
    currentStep = getNextStep();
}

uint8_t SteampunquencerScript::getNextStep() {
    uint8_t nextStep = currentStep;
    
    switch (direction) {
        case DIR_FORWARD:
            nextStep = (currentStep + 1) % 8;
            break;
            
        case DIR_REVERSE:
            nextStep = (currentStep == 0) ? 7 : currentStep - 1;
            break;
            
        case DIR_PENDULUM:
            if (pendulumDirection == 1) {
                // Moving forward
                if (currentStep == 7) {
                    pendulumDirection = -1;
                    nextStep = 6;
                } else {
                    nextStep = currentStep + 1;
                }
            } else {
                // Moving backward
                if (currentStep == 0) {
                    pendulumDirection = 1;
                    nextStep = 1;
                } else {
                    nextStep = currentStep - 1;
                }
            }
            break;
            
        case DIR_RANDOM:
            // Random walk: choose adjacent step or stay
            int8_t delta = random(-1, 2); // -1, 0, or 1
            nextStep = (currentStep + delta + 8) % 8;
            break;
    }
    
    return nextStep;
}

void SteampunquencerScript::updatePortamento() {
    unsigned long currentMicros = micros();
    unsigned long elapsed = currentMicros - slideStartMicros;
    
    if (elapsed >= (portamentoTimeMs * 1000UL)) {
        // Portamento complete
        currentCV = targetCV;
    } else {
        // Linear interpolation
        float progress = (float)elapsed / (float)(portamentoTimeMs * 1000UL);
        currentCV = currentCV + (targetCV - currentCV) * progress * 0.1f; // Smooth approach
    }
    
    outputCV(currentCV);
}

void SteampunquencerScript::stop() {
    // Turn off outputs
    outputCV(0.0f);
    outputGate(false);
    gateHigh = false;
    steamTrigger = false;
}

void SteampunquencerScript::setGlobalTempo(float bpm) {
    if (bpm < 20.0f) bpm = 20.0f;
    if (bpm > 300.0f) bpm = 300.0f;
    stepDurationMicros = (unsigned long)((60.0f / bpm) * 1000000.0f);
}

void SteampunquencerScript::setStepValue(uint8_t step, int8_t semitones) {
    if (step >= 8) return;
    if (semitones < -12) semitones = -12;
    if (semitones > 12) semitones = 12;
    stepValues[step] = semitones;
}

void SteampunquencerScript::setStepDuration(uint8_t step, uint8_t beats) {
    if (step >= 8) return;
    if (beats < 1) beats = 1;
    if (beats > 8) beats = 8;
    stepDurations[step] = beats;
}

void SteampunquencerScript::setStepGateMode(uint8_t step, GateMode mode) {
    if (step >= 8) return;
    stepGateModes[step] = mode;
}

void SteampunquencerScript::setRootNote(uint8_t note) {
    if (note < 12) {
        rootNote = note;
    }
}

void SteampunquencerScript::setScale(uint8_t scale) {
    if (scale <= 1) {
        scaleType = scale;
    }
}

void SteampunquencerScript::setDirection(DirectionMode dir) {
    direction = dir;
}

void SteampunquencerScript::setCurrentStep(uint8_t step) {
    if (step < 8) {
        currentStep = step;
    }
}

void SteampunquencerScript::setPortamentoTime(uint16_t ms) {
    if (ms < 10) ms = 10;
    if (ms > 500) ms = 500;
    portamentoTimeMs = ms;
}

float SteampunquencerScript::calculateCVVoltage(int8_t stepValue) {
    // Direct semitone mapping (chromatic)
    // C4 (MIDI 60) = 2V, each semitone = 1/12 V
    float voltage = 2.0f + (stepValue / 12.0f);
    
    // Add root note offset
    voltage += (rootNote / 12.0f);
    
    // Clamp to 0-5V range
    if (voltage < 0.0f) voltage = 0.0f;
    if (voltage > 5.0f) voltage = 5.0f;
    
    return voltage;
}

uint16_t SteampunquencerScript::voltageToDACValue(float volts) {
    if (volts < 0.0f) volts = 0.0f;
    if (volts > DAC_MAX_VOLTAGE) volts = DAC_MAX_VOLTAGE;
    
    uint16_t dacValue = (uint16_t)((volts / DAC_MAX_VOLTAGE) * DAC_MAX_VALUE);
    if (dacValue > DAC_MAX_VALUE) dacValue = DAC_MAX_VALUE;
    
    return dacValue;
}

void SteampunquencerScript::outputCV(float volts) {
    if (dacCVInitialized) {
        uint16_t dacValue = voltageToDACValue(volts);
        dacCV.setVoltage(dacValue, false);
    }
}

void SteampunquencerScript::outputGate(bool high) {
    if (dacGateInitialized) {
        float voltage = high ? GATE_HIGH_VOLTAGE : GATE_LOW_VOLTAGE;
        uint16_t dacValue = voltageToDACValue(voltage);
        dacGate.setVoltage(dacValue, false);
    }
}

void SteampunquencerScript::getDisplayText(char* buffer, size_t bufferSize) {
    const char* noteNames[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
    const char* dirNames[] = {"FWD", "REV", "PEND", "RND"};
    
    snprintf(buffer, bufferSize,
             "SPNQ\nStep:%d/%d\nRoot:%s %s",
             currentStep + 1,
             beatCounter + 1,
             noteNames[rootNote],
             dirNames[direction]);
}
