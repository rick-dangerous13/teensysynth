/**
 * LFO Script Implementation
 */

#include "lfo_script.h"
#include <math.h>

LFOScript::LFOScript() 
    : dac1(nullptr)
    , dac2(nullptr)
    , frequency(1.0f)
    , level(5.0f)
    , waveform(0)  // Sine
    , phase(0.0f)
    , currentValue(0.0f)
    , lastUpdateMicros(0)
    , dacInitialized(false) {
}

bool LFOScript::begin() {
    // DACs are initialized externally and shared
    // LFO uses DAC1 for CV output
    dacInitialized = false;  // Will be set by setDAC()
    
    lastUpdateMicros = micros();
    phase = 0.0f;
    
    return true;
}

void LFOScript::setDAC(Adafruit_MCP4725* dac1Ptr, Adafruit_MCP4725* dac2Ptr) {
    dac1 = dac1Ptr;
    dac2 = dac2Ptr;
    if (dac1 != nullptr) {
        dacInitialized = true;
        // Set initial output to 0V
        dac1->setVoltage(0, false);
        Serial.println("LFO: Using MCP4725 DAC1 for CV output");
    } else {
        dacInitialized = false;
        Serial.println("LFO: WARNING - No DAC available, running without CV output");
    }
}

void LFOScript::update() {
    // Calculate time delta
    unsigned long currentMicros = micros();
    unsigned long deltaMicros = currentMicros - lastUpdateMicros;
    lastUpdateMicros = currentMicros;
    
    // Update phase based on frequency
    float deltaTime = deltaMicros / 1000000.0f;  // Convert to seconds
    float phaseIncrement = 2.0f * PI * frequency * deltaTime;
    phase += phaseIncrement;
    
    // Wrap phase to 0-2π
    while (phase >= 2.0f * PI) {
        phase -= 2.0f * PI;
    }
    
    // Calculate waveform value (0.0 to 1.0)
    currentValue = calculateWaveform();
    
    // Convert to voltage and output to DAC
    float outputVoltage = currentValue * level;
    
    // Update MCP4725 DAC1
    if (dacInitialized && dac1 != nullptr) {
        uint16_t dacValue = voltageToDACValue(outputVoltage);
        dac1->setVoltage(dacValue, false);
    }
}

void LFOScript::stop() {
    if (dacInitialized && dac1 != nullptr) {
        // Set output to 0V
        dac1->setVoltage(0, false);
    }
}

void LFOScript::setFrequency(float hz) {
    // Clamp to reasonable range
    if (hz < 0.01f) hz = 0.01f;
    if (hz > 100.0f) hz = 100.0f;
    frequency = hz;
}

void LFOScript::setLevel(float volts) {
    // Clamp to 0-5V
    if (volts < 0.0f) volts = 0.0f;
    if (volts > 5.0f) volts = 5.0f;
    level = volts;
}

void LFOScript::setWaveform(uint8_t type) {
    if (type <= 3) {
        waveform = type;
    }
}

float LFOScript::calculateWaveform() {
    switch (waveform) {
        case 0: // Sine
            return (sin(phase) + 1.0f) * 0.5f;
        
        case 1: // Triangle
            {
                float normalized = phase / (2.0f * PI);
                if (normalized < 0.5f) {
                    return normalized * 2.0f;  // Rising
                } else {
                    return 2.0f - (normalized * 2.0f);  // Falling
                }
            }
        
        case 2: // Square
            return (phase < PI) ? 1.0f : 0.0f;
        
        case 3: // Sawtooth
            return phase / (2.0f * PI);
        
        default:
            return 0.0f;
    }
}

uint16_t LFOScript::voltageToDACValue(float volts) {
    // MCP4725 is 12-bit (0-4095)
    // Map 0-5V to 0-4095
    if (volts < 0.0f) volts = 0.0f;
    if (volts > 5.0f) volts = 5.0f;
    return (uint16_t)((volts / 5.0f) * DAC_MAX_VALUE);
}

void LFOScript::getDisplayText(char* buffer, size_t bufferSize) {
    const char* waveNames[] = {"SINE", "TRI", "SQR", "SAW"};
    
    snprintf(buffer, bufferSize,
             "LFO\nFreq: %.2f Hz\nLevel: %.1f V\nWave: %s",
             frequency, level, waveNames[waveform]);
}
