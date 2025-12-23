/**
 * Poliquencer Script Implementation
 * 
 * Metropolix-inspired sequencer with gate modes and direction control
 * Artistic sequencer with visual feedback
 */

#include "poliquencer_script.h"
#include "chord_sequencer_script.h"

// Scale definitions (semitones from root)
const int8_t PoliquencerScript::majorScale[7] = {0, 2, 4, 5, 7, 9, 11};
const int8_t PoliquencerScript::minorScale[7] = {0, 2, 3, 5, 7, 8, 10};

PoliquencerScript::PoliquencerScript()
    : dac1(nullptr)
    , dac2(nullptr)
    , dacInitialized(false)
    , chordSequencer(nullptr)
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
    
    // Initialize steps with interesting pattern
    // Pattern: Root, 3rd, 5th, octave, 7th, 5th, 4th, 2nd
    int8_t pattern[8] = {0, 4, 7, 12, 10, 7, 5, 2};
    for (int i = 0; i < 8; i++) {
        stepValues[i] = pattern[i];
        stepDurations[i] = 1;  // 1 beat per step by default
        stepGateModes[i] = GATE_NORMAL;  // All switches in upper position
    }
}

bool PoliquencerScript::begin() {
    // DAC is initialized externally and shared
    dacInitialized = false;  // Will be set by setDAC()
    
    lastStepMicros = micros();
    
    // Calculate initial CV
    targetCV = calculateCVVoltage(stepValues[currentStep]);
    currentCV = targetCV;
    
    return true;
}

void PoliquencerScript::setDAC(Adafruit_MCP4725* dac1Ptr, Adafruit_MCP4725* dac2Ptr) {
    dac1 = dac1Ptr;
    dac2 = dac2Ptr;
    
    Serial.println("Poliquencer: setDAC() called");
    Serial.print("  dac1Ptr: ");
    Serial.println((unsigned long)dac1Ptr, HEX);
    Serial.print("  dac2Ptr: ");
    Serial.println((unsigned long)dac2Ptr, HEX);
    
    if (dac1 != nullptr && dac2 != nullptr) {
        dacInitialized = true;
        
        // Initialize pitch CV (DAC1) with current step voltage
        float voltage = calculateCVVoltage(stepValues[currentStep]);
        uint16_t dacValue = voltageToDACValue(voltage);
        
        Serial.println("Poliquencer: Initializing DAC outputs (via TCA9548A multiplexer)");
        Serial.print("  Setting DAC1 (Pitch, Channel 0) to ");
        Serial.print(voltage);
        Serial.print("V (DAC value: ");
        Serial.print(dacValue);
        Serial.println(")");
        
        // Note: Actual multiplexer channel selection happens in outputCV/outputGate
        // Here we just initialize the DAC values
        dac1->setVoltage(dacValue, false);
        
        // Initialize gate CV (DAC2) to low (Channel 1)
        Serial.println("  Setting DAC2 (Gate, Channel 1) to 0V");
        dac2->setVoltage(0, false);
        
        Serial.println("✓ Poliquencer: Using 2x MCP4725 DACs with TCA9548A multiplexer");
        Serial.println("  DAC1 (0x60, Channel 0): Pitch CV output");
        Serial.println("  DAC2 (0x60, Channel 1): Gate CV output");
    } else {
        dacInitialized = false;
        Serial.println("✗ Poliquencer: WARNING - No DAC available");
        if (dac1 == nullptr) Serial.println("  dac1 is NULL");
        if (dac2 == nullptr) Serial.println("  dac2 is NULL");
        Serial.println("  Running without CV output");
    }
}

void PoliquencerScript::setChordSequencer(ChordSequencerScript* chordSeq) {
    chordSequencer = chordSeq;
    if (chordSeq != nullptr) {
        Serial.println("Poliquencer: Chord sequencer attached for pitch quantization");
    } else {
        Serial.println("Poliquencer: Chord sequencer detached (will output chromatic)");
    }
}

void PoliquencerScript::update() {
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
        if ((currentMicros - gateOnMicros) >= GATE_LENGTH_MICROS) {
            gateHigh = false;
            outputGate(false);
        }
    }
}

void PoliquencerScript::advanceStep() {
    currentStep = getNextStep();
}

uint8_t PoliquencerScript::getNextStep() {
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

void PoliquencerScript::updatePortamento() {
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

void PoliquencerScript::stop() {
    // Turn off all outputs
    if (dacInitialized && dac1 != nullptr && dac2 != nullptr) {
        dac1->setVoltage(0, false);  // Set CV to 0V
        dac2->setVoltage(0, false);  // Set gate to 0V
    }
    gateHigh = false;
    steamTrigger = false;
}

void PoliquencerScript::setGlobalTempo(float bpm) {
    if (bpm < 20.0f) bpm = 20.0f;
    if (bpm > 300.0f) bpm = 300.0f;
    stepDurationMicros = (unsigned long)((60.0f / bpm) * 1000000.0f);
}

void PoliquencerScript::setStepValue(uint8_t step, int8_t semitones) {
    if (step >= 8) return;
    if (semitones < -12) semitones = -12;
    if (semitones > 12) semitones = 12;
    stepValues[step] = semitones;
}

void PoliquencerScript::setStepDuration(uint8_t step, uint8_t beats) {
    if (step >= 8) return;
    if (beats < 1) beats = 1;
    if (beats > 8) beats = 8;
    stepDurations[step] = beats;
}

void PoliquencerScript::setStepGateMode(uint8_t step, GateMode mode) {
    if (step >= 8) return;
    stepGateModes[step] = mode;
}

void PoliquencerScript::setRootNote(uint8_t note) {
    if (note < 12) {
        rootNote = note;
    }
}

void PoliquencerScript::setScale(uint8_t scale) {
    if (scale <= 1) {
        scaleType = scale;
    }
}

void PoliquencerScript::setDirection(DirectionMode dir) {
    direction = dir;
}

void PoliquencerScript::setCurrentStep(uint8_t step) {
    if (step < 8) {
        currentStep = step;
    }
}

void PoliquencerScript::setPortamentoTime(uint16_t ms) {
    if (ms < 10) ms = 10;
    if (ms > 500) ms = 500;
    portamentoTimeMs = ms;
}

float PoliquencerScript::calculateCVVoltage(int8_t stepValue) {
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

float PoliquencerScript::quantizeToScale(float volts, uint8_t rootNote, const int8_t scaleNotes[7]) {
    // Quantize voltage to the nearest scale degree
    // volts: 0-5V range (0V = C3, 2V = reference point for C4)
    // rootNote: 0-11 (C to B)
    // scaleNotes: 7 semitone offsets from root
    
    // Convert voltage to semitones from C3 (0V reference, but we use 2V = C4 offset)
    float semitones = (volts - 2.0f) * 12.0f;
    
    // Calculate the target semitone within the octave, accounting for root note
    float targetSemitone = semitones - rootNote;
    
    // Normalize to 0-12 range to find position within the octave
    float octavePosition = fmod(targetSemitone, 12.0f);
    if (octavePosition < 0.0f) octavePosition += 12.0f;
    
    // Find the closest scale degree
    int8_t closestDegree = scaleNotes[0];
    float closestDistance = fabsf(octavePosition - scaleNotes[0]);
    
    for (int i = 1; i < 7; i++) {
        float distance = fabsf(octavePosition - scaleNotes[i]);
        if (distance < closestDistance) {
            closestDistance = distance;
            closestDegree = scaleNotes[i];
        }
    }
    
    // Convert back to voltage
    float quantizedSemitones = (semitones - octavePosition) + closestDegree + rootNote;
    float quantizedVoltage = 2.0f + (quantizedSemitones / 12.0f);
    
    // Clamp to 0-5V range
    if (quantizedVoltage < 0.0f) quantizedVoltage = 0.0f;
    if (quantizedVoltage > 5.0f) quantizedVoltage = 5.0f;
    
    return quantizedVoltage;
}

uint16_t PoliquencerScript::voltageToDACValue(float volts) {
    // MCP4725 is 12-bit (0-4095)
    // Map 0-5V to 0-4095
    if (volts < 0.0f) volts = 0.0f;
    if (volts > 5.0f) volts = 5.0f;
    return (uint16_t)((volts / 5.0f) * DAC_MAX_VALUE);
}

void PoliquencerScript::outputCV(float volts) {
    if (!dacInitialized) {
        return;  // Silent fail if DACs not initialized
    }
    
    if (dac1 == nullptr) {
        Serial.println("ERROR: outputCV() called but dac1 is NULL");
        return;
    }
    
    // Apply quantization if chord sequencer is available and running
    float outputVolts = volts;
    if (chordSequencer != nullptr) {
        // Get active chord info
        uint8_t chordSlot = chordSequencer->getCurrentChordSlot();
        uint8_t chordCount = chordSequencer->getChordCount();
        
        if (chordCount > 0 && chordSlot < chordCount) {
            // Get scale notes for quantization (scale includes correct root and intervals)
            ScaleInfo scale;
            chordSequencer->getCurrentScale(&scale);
            
            // Quantize to the active chord's scale
            // Use scale.rootNote since getCurrentScale() already sets it to the current chord's root
            outputVolts = quantizeToScale(volts, scale.rootNote, (const int8_t*)scale.notes);
            
            // Debug: Log when quantization occurs
            // Serial.print("Quantized: ");
            // Serial.print(volts, 2);
            // Serial.print("V -> ");
            // Serial.println(outputVolts, 2);
        }
    }
    
    // Select multiplexer channel for DAC1 (Pitch CV)
    Wire.beginTransmission(TCA9548A_ADDR);
    Wire.write(1 << MCP4725_CHANNEL_1);  // Select channel 0
    Wire.endTransmission();
    
    // Update pitch CV on DAC1 with quantized value
    uint16_t dacValue = voltageToDACValue(outputVolts);
    dac1->setVoltage(dacValue, false);
    
    // Debug: Log CV changes
    static uint16_t lastDacValue = 0xFFFF;
    if (dacValue != lastDacValue) {
        Serial.print("DAC1 CV: ");
        Serial.print(outputVolts, 2);
        Serial.print("V (value: ");
        Serial.print(dacValue);
        Serial.println(")");
        lastDacValue = dacValue;
    }
}

void PoliquencerScript::outputGate(bool high) {
    if (!dacInitialized) {
        return;  // Silent fail if DACs not initialized
    }
    
    if (dac2 == nullptr) {
        Serial.println("ERROR: outputGate() called but dac2 is NULL");
        return;
    }
    
    // Select multiplexer channel for DAC2 (Gate CV)
    Wire.beginTransmission(TCA9548A_ADDR);
    Wire.write(1 << MCP4725_CHANNEL_2);  // Select channel 1
    Wire.endTransmission();
    
    // Output gate on DAC2: 5V for high, 0V for low
    uint16_t gateValue = high ? DAC_MAX_VALUE : 0;
    dac2->setVoltage(gateValue, false);
    
    // Debug: Log gate changes
    Serial.print("DAC2 Gate: ");
    Serial.println(high ? "HIGH (5V)" : "LOW (0V)");
}

void PoliquencerScript::getDisplayText(char* buffer, size_t bufferSize) {
    const char* noteNames[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
    const char* dirNames[] = {"FWD", "REV", "PEND", "RND"};
    
    snprintf(buffer, bufferSize,
             "SPNQ\nStep:%d/%d\nRoot:%s %s",
             currentStep + 1,
             beatCounter + 1,
             noteNames[rootNote],
             dirNames[direction]);
}
