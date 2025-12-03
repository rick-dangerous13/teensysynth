/**
 * DAC8568 Driver
 * 
 * Texas Instruments DAC8568 - 16-bit, 8-channel DAC with SPI interface
 * Voltage output: 0-5V (with 5V reference)
 * Resolution: 16-bit (0-65535)
 */

#ifndef DAC8568_H
#define DAC8568_H

#include <Arduino.h>
#include <SPI.h>

// DAC8568 Command definitions
#define DAC8568_CMD_WRITE_UPDATE      0x03  // Write to input register and update DAC
#define DAC8568_CMD_UPDATE_ALL        0x07  // Update all DAC channels
#define DAC8568_CMD_WRITE_UPDATE_ALL  0x02  // Write to all input registers and update all DACs
#define DAC8568_CMD_POWER_DOWN        0x04  // Power down/up DAC channels
#define DAC8568_CMD_RESET             0x05  // Reset (power-on reset)
#define DAC8568_CMD_LDAC_SETUP        0x06  // LDAC register setup
#define DAC8568_CMD_INTERNAL_REF      0x08  // Internal reference setup

// Channel addresses (A2 A1 A0)
#define DAC8568_CHANNEL_A  0x00
#define DAC8568_CHANNEL_B  0x01
#define DAC8568_CHANNEL_C  0x02
#define DAC8568_CHANNEL_D  0x03
#define DAC8568_CHANNEL_E  0x04
#define DAC8568_CHANNEL_F  0x05
#define DAC8568_CHANNEL_G  0x06
#define DAC8568_CHANNEL_H  0x07
#define DAC8568_CHANNEL_ALL 0x0F

class DAC8568 {
public:
    /**
     * Constructor
     * @param csPin Chip select pin for SPI
     * @param refVoltage Reference voltage (default 5.0V for external ref)
     */
    DAC8568(uint8_t csPin, float refVoltage = 5.0f);
    
    /**
     * Initialize the DAC
     * @return true if successful
     */
    bool begin();
    
    /**
     * Set raw 16-bit value on a specific channel
     * @param channel Channel number (0-7 or DAC8568_CHANNEL_*)
     * @param value Raw 16-bit value (0-65535)
     */
    void setChannel(uint8_t channel, uint16_t value);
    
    /**
     * Set voltage on a specific channel
     * @param channel Channel number (0-7)
     * @param voltage Desired voltage (0.0 to refVoltage)
     */
    void setVoltage(uint8_t channel, float voltage);
    
    /**
     * Set all channels to the same raw value
     * @param value Raw 16-bit value (0-65535)
     */
    void setAllChannels(uint16_t value);
    
    /**
     * Set all channels to the same voltage
     * @param voltage Desired voltage (0.0 to refVoltage)
     */
    void setAllVoltages(float voltage);
    
    /**
     * Reset all DAC channels to 0V
     */
    void reset();
    
    /**
     * Convert voltage to 16-bit DAC value
     * @param voltage Input voltage (0.0 to refVoltage)
     * @return 16-bit DAC value (0-65535)
     */
    uint16_t voltageToDACValue(float voltage);
    
    /**
     * Get reference voltage
     */
    float getRefVoltage() const { return _refVoltage; }

private:
    uint8_t _csPin;
    float _refVoltage;
    bool _initialized;
    
    /**
     * Send 24-bit command to DAC
     * @param command Command byte (bits 23-20: command, bits 19-16: address)
     * @param data 16-bit data value
     */
    void sendCommand(uint8_t command, uint8_t address, uint16_t data);
};

#endif // DAC8568_H
