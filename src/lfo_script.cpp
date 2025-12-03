/**
 * LFO Script Implementation
 */

#include "lfo_script.h"
#include <math.h>

LFOScript::LFOScript() 
    : dac(nullptr)
    , frequency(1.0f)
    , level(5.0f)
    , waveform(0)  // Sine
    , phase(0.0f)
    , currentValue(0.0f)
    , lastUpdateMicros(0)
    , dacInitialized(false) {
}

bool LFOScript::begin() {
    // DAC is initialized externally and shared
    // LFO uses channel 0 (DAC_CH_STEP1) for output
    dacInitialized = false;  // Will be set by setDAC()
    
    lastUpdateMicros = micros();
    phase = 0.0f;
    
    return true;
}

void LFOScript::setDAC(DAC8568* dacPtr) {
    dac = dacPtr;
    if (dac != nullptr) {
        dacInitialized = true;
        // Set initial output to 0V on channel 0
        dac->setVoltage(DAC_CH_STEP1, 0.0f);
        Serial.println("LFO: Using DAC8568 channel 0 for CV output");
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
    
    // Update DAC channel 0
    if (dacInitialized && dac != nullptr) {
        dac->setVoltage(DAC_CH_STEP1, outputVoltage);
    }
}

void LFOScript::stop() {
    if (dacInitialized && dac != nullptr) {
        // Set output to 0V on channel 0
        dac->setVoltage(DAC_CH_STEP1, 0.0f);
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
    // DAC8568 is 16-bit (0-65535)
    // Uses DAC8568 class method internally, this is kept for compatibility
    if (dac != nullptr) {
        return dac->voltageToDACValue(volts);
    }
    return 0;
}

void LFOScript::getDisplayText(char* buffer, size_t bufferSize) {
    const char* waveNames[] = {"SINE", "TRI", "SQR", "SAW"};
    
    snprintf(buffer, bufferSize,
             "LFO\nFreq: %.2f Hz\nLevel: %.1f V\nWave: %s",
             frequency, level, waveNames[waveform]);
}
