/**
 * LFO Script Implementation
 */

#include "lfo_script.h"
#include <Wire.h>
#include <math.h>

LFOScript::LFOScript() 
    : frequency(1.0f)
    , level(5.0f)
    , waveform(0)  // Sine
    , phase(0.0f)
    , currentValue(0.0f)
    , lastUpdateMicros(0)
    , dacInitialized(false) {
}

bool LFOScript::begin() {
    // Initialize I2C
    Wire.begin();
    
    // Initialize MCP4725 DAC
    if (!dac.begin(MCP4725_CV_ADDR)) {
        Serial.println("LFO: WARNING - MCP4725 DAC not found, running without CV output");
        Serial.println("     Check wiring: VDD->5V, GND->GND, SDA->18, SCL->19");
        dacInitialized = false;
        // Continue anyway - allow visual display without hardware
    } else {
        dacInitialized = true;
        // Set initial output to 0V
        dac.setVoltage(0, false);
        Serial.println("LFO: Initialized successfully with MCP4725 DAC");
        Serial.println("     CV output available on VOUT pin (connect to TRRS jack)");
    }
    
    lastUpdateMicros = micros();
    phase = 0.0f;
    
    return true;  // Always return true to allow running without DAC
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
    uint16_t dacValue = voltageToDACValue(outputVoltage);
    
    // Update DAC (false = don't write to EEPROM)
    if (dacInitialized) {
        dac.setVoltage(dacValue, false);
    }
}

void LFOScript::stop() {
    if (dacInitialized) {
        // Set output to 0V
        dac.setVoltage(0, false);
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
    // Assuming VDD = 5V reference
    if (volts < 0.0f) volts = 0.0f;
    if (volts > DAC_MAX_VOLTAGE) volts = DAC_MAX_VOLTAGE;
    
    uint16_t dacValue = (uint16_t)((volts / DAC_MAX_VOLTAGE) * DAC_MAX_VALUE);
    if (dacValue > DAC_MAX_VALUE) dacValue = DAC_MAX_VALUE;
    
    return dacValue;
}

void LFOScript::getDisplayText(char* buffer, size_t bufferSize) {
    const char* waveNames[] = {"SINE", "TRI", "SQR", "SAW"};
    
    snprintf(buffer, bufferSize,
             "LFO\nFreq: %.2f Hz\nLevel: %.1f V\nWave: %s",
             frequency, level, waveNames[waveform]);
}
