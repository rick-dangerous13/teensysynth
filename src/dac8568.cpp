/**
 * DAC8568 Driver Implementation
 */

#include "dac8568.h"

DAC8568::DAC8568(uint8_t csPin, float refVoltage, uint8_t rstPin)
    : _csPin(csPin)
    , _rstPin(rstPin)
    , _refVoltage(refVoltage)
    , _initialized(false) {
}

bool DAC8568::begin() {
    // Initialize CS pin - keep HIGH (inactive)
    pinMode(_csPin, OUTPUT);
    digitalWrite(_csPin, HIGH);
    
    // Initialize RST pin if provided
    if (_rstPin != 255) {
        pinMode(_rstPin, OUTPUT);
        digitalWrite(_rstPin, LOW);   // Assert reset
        delay(10);
        digitalWrite(_rstPin, HIGH);  // Release reset
        delay(10);
        Serial.println("DAC8568: Hardware reset performed");
    }
    
    // Initialize SPI1 (separate bus from display)
    SPI1.setMOSI(26);  // DAC_MOSI
    SPI1.setSCK(27);   // DAC_SCK
    SPI1.begin();
    
    delay(10);
    
    Serial.println("DAC8568: Initializing on SPI1 (Pin 26/27)...");
    Serial.print("DAC8568: CS pin = ");
    Serial.println(_csPin);
    
    // Perform software reset
    Serial.println("DAC8568: Sending software reset...");
    sendCommand(DAC8568_CMD_RESET, 0x00, 0x0000);
    delay(10);
    
    // DISABLE internal reference - use external reference (VDD)
    // This makes the DAC use its 5V power supply as the reference
    // BOOST-DAC8568 module may be designed for external reference mode
    Serial.println("DAC8568: Using EXTERNAL reference (VDD=5V)...");
    sendCommand(DAC8568_CMD_INTERNAL_REF, 0x00, 0x0000);  // 0x0000 = disable internal, use external
    delay(50);
    
    // Power up all DAC channels (0x0000 = all channels powered up)
    Serial.println("DAC8568: Powering up all channels...");
    sendCommand(DAC8568_CMD_POWER_DOWN, 0x00, 0x0000);
    delay(50);  // Longer delay for DAC outputs to stabilize
    
    // Set all channels to 0V
    Serial.println("DAC8568: Setting all channels to 0V...");
    sendCommand(DAC8568_CMD_WRITE_UPDATE_ALL, DAC8568_CHANNEL_ALL, 0x0000);
    delay(10);
    
    _initialized = true;
    
    Serial.println("DAC8568: Initialization complete on SPI1!");
    Serial.println("DAC8568: Ready - isolated from display bus");
    
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
    static unsigned long lastDebug = 0;
    uint16_t value = voltageToDACValue(voltage);
    
    // Debug output every 1 second
    if (millis() - lastDebug > 1000) {
        Serial.print("DAC CH");
        Serial.print(channel);
        Serial.print(": ");
        Serial.print(voltage, 3);
        Serial.print("V (raw=");
        Serial.print(value);
        Serial.println(")");
        lastDebug = millis();
    }
    
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
    
    // Debug: Print every command being sent
    Serial.print("SPI→ 0x");
    if (byte1 < 0x10) Serial.print("0");
    Serial.print(byte1, HEX);
    Serial.print(" 0x");
    if (byte2 < 0x10) Serial.print("0");
    Serial.print(byte2, HEX);
    Serial.print(" 0x");
    if (byte3 < 0x10) Serial.print("0");
    Serial.print(byte3, HEX);
    Serial.print(" [CMD:");
    Serial.print(command);
    Serial.print(" CH:");
    Serial.print(address);
    Serial.print(" DATA:");
    Serial.print(data);
    Serial.println("]");
    
    // SPI settings: Mode 1 (CPOL=0, CPHA=1), MSB first, max 50MHz
    SPISettings settings(20000000, MSBFIRST, SPI_MODE1);
    
    SPI1.beginTransaction(settings);  // Use SPI1, not SPI
    digitalWrite(_csPin, LOW);   // Select DAC
    
    SPI1.transfer(byte1);  // Command + Address
    SPI1.transfer(byte2);  // Data high byte
    SPI1.transfer(byte3);  // Data low byte
    
    digitalWrite(_csPin, HIGH);  // Deselect DAC
    SPI1.endTransaction();
}
