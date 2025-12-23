/**
 * Polyphonion Configuration
 * 
 * Hardware pin definitions and system constants
 */

#ifndef CONFIG_H
#define CONFIG_H

// ============================================================================
// Display Configuration (ILI9488)
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

// Display dimensions (landscape on ILI9488)
#define SCREEN_WIDTH   480
#define SCREEN_HEIGHT  320

// ============================================================================
// Button Configuration
// ============================================================================
#define BTN_OK    2    // OK button pin (active LOW with pull-up)
#define BTN_OK2   15   // Secondary OK button pin (alternative)
#define BTN_BACK  3    // Back button pin (active LOW with pull-up)

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
// Touchscreen Configuration (XPT2046)
// ============================================================================
#define TOUCH_CS      7    // Touch chip select
// TOUCH_IRQ not connected - using polling mode

// Touch calibration values (adjust based on your display orientation)
// For rotation(3) landscape mode with USB on right
// Based on calibration: Left=3500, Right=280, Top=3560, Bottom=320
// Swapped MIN/MAX to invert mapping (raw decreases left-to-right, top-to-bottom)
#define TS_MINX       3500   // Left edge (high raw value)
#define TS_MINY       3560   // Top edge (high raw value)
#define TS_MAXX       280    // Right edge (low raw value)
#define TS_MAXY       320    // Bottom edge (low raw value)

// Touch pins:
//   T_IRQ  -> Not connected (or Pin 255 for polling mode)
//   T_DO   -> Pin 12 (MISO - shared with display)
//   T_DIN  -> Pin 11 (MOSI - shared with display)
//   T_CS   -> Pin 7 (dedicated)
//   T_CLK  -> Pin 13 (SCK - shared with display)

// ============================================================================
// ============================================================================
// MCP4725 DAC Configuration (I2C CV Outputs with TCA9548A Multiplexer)
// ============================================================================
// Using 2x MCP4725 12-bit I2C DACs with TCA9548A I2C multiplexer
// This allows both DACs to use the same I2C address (0x60) on different channels
//
// HARDWARE SETUP (TCA9548A Multiplexer Required):
// Your MCP4725 boards have NO address pins (A0/A1/A2), so we use a multiplexer.
// 
// Wiring:
//   Teensy Pin 18 (SDA) ──→ TCA9548A SDA
//   Teensy Pin 19 (SCL) ──→ TCA9548A SCL
//   Teensy 5V ──→ TCA9548A VIN
//   Teensy GND ──→ TCA9548A GND, A0, A1, A2
// 
//   TCA9548A Channel 0 (SD0/SC0) ──→ MCP4725 #1 SDA/SCL (Pitch CV)
//   TCA9548A Channel 1 (SD1/SC1) ──→ MCP4725 #2 SDA/SCL (Gate CV)
// 
//   Each MCP4725: VCC→5V, GND→GND, OUT→Synth CV Input
//
// See MCP4725_DUAL_DAC_WITH_MULTIPLEXER.md for detailed wiring!

// TCA9548A I2C Multiplexer
#define TCA9548A_ADDR         0x70  // Multiplexer address (A0=GND, A1=GND, A2=GND)

// MCP4725 DAC Addresses and Multiplexer Channels
#define MCP4725_ADDR_1        0x60  // Pitch CV (on multiplexer channel 0)
#define MCP4725_ADDR_2        0x60  // Gate CV (on multiplexer channel 1 - same address, different channel!)
#define MCP4725_CHANNEL_1     0     // Channel 0 for Pitch CV
#define MCP4725_CHANNEL_2     1     // Channel 1 for Gate CV

// DAC specifications
#define DAC_MAX_VALUE         4095      // 12-bit DAC (0-4095)
#define DAC_MAX_VOLTAGE       5.0f      // Maximum output voltage
#define GATE_HIGH_VOLTAGE     5.0f      // Gate high voltage
#define GATE_LOW_VOLTAGE      0.0f      // Gate low voltage

// ============================================================================
// Script Manager Configuration
// ============================================================================
#define MAX_SCRIPTS   4    // Maximum concurrent scripts
// Chord sequencer configuration
#define MAX_CHORD_SLOTS 8   // Maximum chords in a progression

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
