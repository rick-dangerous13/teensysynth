/**
 * I2C Scanner Test
 * Upload this to check if both MCP4725 DACs are detected
 */

#include <Arduino.h>
#include <Wire.h>

void setup() {
    Serial.begin(115200);
    delay(2000);
    
    Serial.println("\nI2C Scanner Starting...");
    Wire.begin();
    
    Serial.println("Scanning I2C bus...");
    
    byte count = 0;
    for (byte address = 1; address < 127; address++) {
        Wire.beginTransmission(address);
        byte error = Wire.endTransmission();
        
        if (error == 0) {
            Serial.print("Device found at address 0x");
            if (address < 16) Serial.print("0");
            Serial.print(address, HEX);
            Serial.print(" (");
            Serial.print(address);
            Serial.print(")");
            
            // Identify known devices
            if (address == 0x60) Serial.print(" - MCP4725 CV DAC");
            if (address == 0x61) Serial.print(" - MCP4725 Gate DAC");
            if (address == 0x62) Serial.print(" - MCP4725 (alt address)");
            
            Serial.println();
            count++;
        }
    }
    
    Serial.println("Scan complete.");
    Serial.print("Found ");
    Serial.print(count);
    Serial.println(" device(s).");
    
    if (count == 0) {
        Serial.println("\nNO I2C devices found!");
        Serial.println("Check:");
        Serial.println("  - SDA connected to Pin 18");
        Serial.println("  - SCL connected to Pin 19");
        Serial.println("  - Both DACs powered (VCC to 5V)");
        Serial.println("  - Common GND connection");
    }
}

void loop() {
    delay(5000);
    Serial.println("\nPress reset to scan again...");
}
