/**
 * DAC8568 Driver Implementation
 */

#include "dac8568.h"

DAC8568::DAC8568(uint8_t csPin, float refVoltage)
    : _csPin(csPin)
    , _refVoltage(refVoltage)
    , _initialized(false) {
}

bool DAC8568::begin() {
    // Initialize CS pin - keep HIGH (inactive) to avoid bus conflicts
    pinMode(_csPin, OUTPUT);
    digitalWrite(_csPin, HIGH);  // CS is active low - keep deselected
    
    // SPI is shared with display/touch - already initialized by display
    // Don't call SPI.begin() again as it may reset the bus
    
    // Small delay to ensure pin is stable
    delay(1);
    
    // Mark as initialized - DAC commands will only execute if CS pin is valid
    _initialized = true;
    
    Serial.println("DAC8568: CS pin configured on Pin 14");
    Serial.println("DAC8568: Skipping hardware initialization (no detection method)");
    Serial.println("DAC8568: Will send commands when DAC hardware is connected");
    
    return true;
}

void DAC8568::setChannel(uint8_t channel, uint16_t value) {
    if (!_initialized) return;
    if (channel > 7) return;
    
    // Clamp value to 16-bit range
    if (value > 65535) value = 65535;
    
    // Write to input register and update DAC immediately
    sendCommand(DAC8568_CMD_WRITE_UPDATE, channel, value);
}

void DAC8568::setVoltage(uint8_t channel, float voltage) {
    uint16_t value = voltageToDACValue(voltage);
    setChannel(channel, value);
}

void DAC8568::setAllChannels(uint16_t value) {
    if (!_initialized) return;
    
    // Clamp value to 16-bit range
    if (value > 65535) value = 65535;
    
    // Write to all input registers and update all DACs
    sendCommand(DAC8568_CMD_WRITE_UPDATE_ALL, DAC8568_CHANNEL_ALL, value);
}

void DAC8568::setAllVoltages(float voltage) {
    uint16_t value = voltageToDACValue(voltage);
    setAllChannels(value);
}

void DAC8568::reset() {
    // Perform software reset
    sendCommand(DAC8568_CMD_RESET, 0x00, 0x0000);
    delay(1);
    
    // Set all channels to 0V
    sendCommand(DAC8568_CMD_WRITE_UPDATE_ALL, DAC8568_CHANNEL_ALL, 0x0000);
    delay(1);
}

uint16_t DAC8568::voltageToDACValue(float voltage) {
    // Clamp voltage to valid range
    if (voltage < 0.0f) voltage = 0.0f;
    if (voltage > _refVoltage) voltage = _refVoltage;
    
    // Convert to 16-bit value (0-65535)
    uint32_t value = (uint32_t)((voltage / _refVoltage) * 65535.0f);
    
    if (value > 65535) value = 65535;
    
    return (uint16_t)value;
}

void DAC8568::sendCommand(uint8_t command, uint8_t address, uint16_t data) {
    // DAC8568 uses 24-bit frames:
    // Bits 23-20: Command (4 bits)
    // Bits 19-16: Address (4 bits)
    // Bits 15-0:  Data (16 bits)
    
    uint8_t byte1 = (command << 4) | (address & 0x0F);
    uint8_t byte2 = (data >> 8) & 0xFF;
    uint8_t byte3 = data & 0xFF;
    
    // SPI settings: Mode 1 (CPOL=0, CPHA=1), MSB first, max 50MHz
    SPISettings settings(20000000, MSBFIRST, SPI_MODE1);
    
    SPI.beginTransaction(settings);
    digitalWrite(_csPin, LOW);   // Select DAC
    
    SPI.transfer(byte1);  // Command + Address
    SPI.transfer(byte2);  // Data high byte
    SPI.transfer(byte3);  // Data low byte
    
    digitalWrite(_csPin, HIGH);  // Deselect DAC
    SPI.endTransaction();
}
