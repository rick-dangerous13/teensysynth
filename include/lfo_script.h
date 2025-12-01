/**
 * LFO Script
 * 
 * Simple Low-Frequency Oscillator with CV output via MCP4725 DAC
 * Generates sine wave modulation (0-5V) for Eurorack systems
 */

#ifndef LFO_SCRIPT_H
#define LFO_SCRIPT_H

#include <Arduino.h>
#include <Adafruit_MCP4725.h>
#include "config.h"

class LFOScript {
public:
    LFOScript();
    
    bool begin();
    void update();
    void stop();
    
    // Parameter control
    void setFrequency(float hz);     // 0.01 Hz to 100 Hz
    void setLevel(float volts);      // 0.0V to 5.0V
    void setWaveform(uint8_t type);  // 0=Sine, 1=Triangle, 2=Square, 3=Saw
    
    // Getters
    float getFrequency() const { return frequency; }
    float getLevel() const { return level; }
    uint8_t getWaveform() const { return waveform; }
    float getCurrentValue() const { return currentValue; }
    
    // Get formatted output for display
    void getDisplayText(char* buffer, size_t bufferSize);
    
    // Draw waveform indicator
    void drawWaveform(int16_t x, int16_t y, int16_t w, int16_t h);

private:
    Adafruit_MCP4725 dac;
    
    // LFO parameters
    float frequency;        // Frequency in Hz
    float level;           // Output level in Volts (0-5V)
    uint8_t waveform;      // Waveform type
    
    // Runtime state
    float phase;           // Current phase (0.0 to 2*PI)
    float currentValue;    // Current output value (0.0 to 1.0)
    unsigned long lastUpdateMicros;
    
    // DAC output
    bool dacInitialized;
    
    // Waveform calculation
    float calculateWaveform();
    uint16_t voltageToDACValue(float volts);
};

#endif // LFO_SCRIPT_H
