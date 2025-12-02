/**
 * Polyphonion Configuration
 * 
 * Hardware pin definitions and system constants
 */

#ifndef CONFIG_H
#define CONFIG_H

// ============================================================================
// Display Configuration (ILI9341)
// ============================================================================
#ifndef TFT_DC
#define TFT_DC    9
#endif

#ifndef TFT_CS
#define TFT_CS    10
#endif

#ifndef TFT_RST
#define TFT_RST   8
#endif

#ifndef TFT_MOSI
#define TFT_MOSI  11
#endif

#ifndef TFT_SCLK
#define TFT_SCLK  13
#endif

#ifndef TFT_MISO
#define TFT_MISO  12
#endif

// Display dimensions
#define SCREEN_WIDTH   320
#define SCREEN_HEIGHT  240

// ============================================================================
// Button Configuration
// ============================================================================
#define BTN_OK    2    // OK button pin
#define BTN_BACK  3    // Back button pin

// Button debounce settings
#define DEBOUNCE_DELAY 50  // milliseconds

// ============================================================================
// Rotary Encoder Configuration (DEBO ENCODER)
// ============================================================================
#define ENC_CLK       4    // Encoder CLK pin
#define ENC_DT        5    // Encoder DT pin
#define ENC_SW        6    // Encoder switch (push button) pin

// Encoder settings
#define ENC_STEPS_PER_NOTCH  1    // State transitions per detent (DEBO encoder = 1)
#define ENC_DEBOUNCE_MS      2    // Debounce time for encoder transitions

// ============================================================================
// MCP4725 DAC Configuration (Eurorack CV/Gate Output)
// ============================================================================
// CV Output DAC (Address 0x60)
#define MCP4725_CV_ADDR   0x60  // CV output I2C address
// Gate Output DAC (Address 0x61) - requires A0 pin tied to VDD
#define MCP4725_GATE_ADDR 0x61  // Gate output I2C address

// Both use Teensy 4.1 I2C pins (Wire): SDA=18, SCL=19
// 
// MCP4725 #1 (CV) Wiring:
//   MCP4725 VDD  -> Teensy 5V (for 0-5V output range)
//   MCP4725 GND  -> Teensy GND
//   MCP4725 SDA  -> Teensy Pin 18 (SDA)
//   MCP4725 SCL  -> Teensy Pin 19 (SCL)
//   MCP4725 A0   -> GND (sets address to 0x60)
//   MCP4725 VOUT -> TRRS Jack "Left" pin (CV output)
// 
// MCP4725 #2 (Gate) Wiring:
//   MCP4725 VDD  -> Teensy 5V
//   MCP4725 GND  -> Teensy GND
//   MCP4725 SDA  -> Teensy Pin 18 (SDA) - shared I2C bus
//   MCP4725 SCL  -> Teensy Pin 19 (SCL) - shared I2C bus
//   MCP4725 A0   -> VDD (sets address to 0x61)
//   MCP4725 VOUT -> TRRS Jack "Ring" pin (Gate output)
// 
// TRRS Jack Output:
//   Left   -> CV output (0-5V analog)
//   Ring   -> Gate output (0V/5V digital)
//   Sleeve -> GND (common ground)
// 
#define DAC_MAX_VALUE 4095  // 12-bit DAC (0-4095)
#define DAC_MAX_VOLTAGE 5.0 // Maximum output voltage (requires VDD=5V)
#define GATE_HIGH_VOLTAGE 5.0  // Gate high voltage
#define GATE_LOW_VOLTAGE 0.0   // Gate low voltage

// ============================================================================
// Script Manager Configuration
// ============================================================================
#define MAX_SCRIPTS   4    // Maximum concurrent scripts

// ============================================================================
// UI Configuration - Norns-style aesthetics
// ============================================================================
// Colors (16-bit RGB565) - Grayscale palette
#define COLOR_BG           0x0000  // Black background
#define COLOR_FG           0xFFFF  // White foreground (text)
#define COLOR_ACCENT       0xCE59  // Light gray accent (bright)
#define COLOR_DIM          0x6B4D  // Medium gray for inactive elements
#define COLOR_HIGHLIGHT    0xD69A  // Lighter gray highlight
#define COLOR_DIAL         0x5D9F  // Light blue for duration dials (exception to grayscale)

// Font sizes
#define FONT_SMALL    1
#define FONT_MEDIUM   2
#define FONT_LARGE    3

// Layout constants
#define MARGIN        10
#define LINE_HEIGHT   20
#define MENU_ITEM_H   25

// ============================================================================
// System Configuration
// ============================================================================
#define SERIAL_BAUD   115200

// Update intervals (milliseconds)
#define INPUT_UPDATE_INTERVAL   10
#define DISPLAY_UPDATE_INTERVAL 16
#define SCRIPT_UPDATE_INTERVAL  10

// Global clock settings
#define DEFAULT_CLOCK_BPM  120.0f
#define MIN_CLOCK_BPM      20.0f
#define MAX_CLOCK_BPM      300.0f

#endif // CONFIG_H
