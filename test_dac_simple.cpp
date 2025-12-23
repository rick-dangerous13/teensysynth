/**
 * SIMPLE DAC TEST
 * 
 * This bypasses all UI and scripts - just sets DAC Channel A to 2.5V
 * Upload this to test if hardware is working at all
 * 
 * MEASUREMENTS TO TAKE:
 * 1. Module Pin 21 vs Pin 22: Should read +5V (positive!)
 * 2. JP1 pad vs GND: Should read steady 2.5V
 * 3. Chip Pin 1 (VOUTA) vs GND: Should read steady 2.5V
 */

#include <Arduino.h>
#include <SPI.h>

// Pin definitions
#define DAC_CS   16
#define DAC_MOSI 26
#define DAC_SCK  27
#define DAC_RST  17

// DAC8568 Commands
#define CMD_WRITE_UPDATE     0x03
#define CMD_RESET            0x05
#define CMD_INTERNAL_REF     0x08
#define CMD_POWER_DOWN       0x04

void sendDAC(uint8_t cmd, uint8_t addr, uint16_t data) {
    uint8_t b1 = (cmd << 4) | (addr & 0x0F);
    uint8_t b2 = (data >> 8) & 0xFF;
    uint8_t b3 = data & 0xFF;
    
    Serial.print("TX: 0x");
    Serial.print(b1, HEX);
    Serial.print(" 0x");
    Serial.print(b2, HEX);
    Serial.print(" 0x");
    Serial.println(b3, HEX);
    
    SPISettings settings(10000000, MSBFIRST, SPI_MODE1);
    SPI1.beginTransaction(settings);
    digitalWrite(DAC_CS, LOW);
    SPI1.transfer(b1);
    SPI1.transfer(b2);
    SPI1.transfer(b3);
    digitalWrite(DAC_CS, HIGH);
    SPI1.endTransaction();
}

void setup() {
    Serial.begin(115200);
    delay(2000);
    
    Serial.println("\n=== SIMPLE DAC TEST ===");
    Serial.println("This test sets Channel A to 2.5V");
    Serial.println("=======================\n");
    
    // Setup pins
    pinMode(DAC_CS, OUTPUT);
    digitalWrite(DAC_CS, HIGH);
    
    pinMode(DAC_RST, OUTPUT);
    digitalWrite(DAC_RST, LOW);
    delay(10);
    digitalWrite(DAC_RST, HIGH);
    delay(10);
    Serial.println("✓ Hardware reset done");
    
    // Setup SPI1
    SPI1.setMOSI(DAC_MOSI);
    SPI1.setSCK(DAC_SCK);
    SPI1.begin();
    Serial.println("✓ SPI1 initialized");
    
    delay(50);
    
    // Software reset
    Serial.println("\n→ Sending reset...");
    sendDAC(CMD_RESET, 0x00, 0x0000);
    delay(50);
    
    // Enable internal reference
    Serial.println("→ Enabling internal reference...");
    sendDAC(CMD_INTERNAL_REF, 0x00, 0x0001);
    delay(100);  // Wait for reference to stabilize
    
    // Power up all channels
    Serial.println("→ Powering up channels...");
    sendDAC(CMD_POWER_DOWN, 0x00, 0x0000);
    delay(50);
    
    // Set Channel A to 2.5V (mid-scale = 0x8000 = 32768)
    Serial.println("→ Setting Channel A to 2.5V (0x8000)...");
    sendDAC(CMD_WRITE_UPDATE, 0x00, 0x8000);
    delay(10);
    
    Serial.println("\n=== TEST COMPLETE ===");
    Serial.println("Channel A should now output 2.5V");
    Serial.println("Measure JP1 pad with multimeter:");
    Serial.println("  Red probe → JP1 pad");
    Serial.println("  Black probe → GND");
    Serial.println("  Expected: +2.5V DC");
    Serial.println("\nAlso measure power:");
    Serial.println("  Module Pin 21 vs Pin 22");
    Serial.println("  Expected: +5.0V DC (not negative!)");
    Serial.println("=====================\n");
}

void loop() {
    // Toggle between 0V and 5V every 2 seconds for easy measurement
    static unsigned long lastToggle = 0;
    static bool highState = false;
    
    if (millis() - lastToggle > 2000) {
        highState = !highState;
        
        if (highState) {
            Serial.println("\n→ Setting to 5V (0xFFFF)");
            sendDAC(CMD_WRITE_UPDATE, 0x00, 0xFFFF);
        } else {
            Serial.println("\n→ Setting to 0V (0x0000)");
            sendDAC(CMD_WRITE_UPDATE, 0x00, 0x0000);
        }
        
        Serial.print("Measure JP1 now - should read: ");
        Serial.println(highState ? "5.0V" : "0.0V");
        
        lastToggle = millis();
    }
    
    delay(100);
}
