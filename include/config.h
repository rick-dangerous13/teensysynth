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
#define BTN_OK    2    // Primary OK button pin
#define BTN_OK2   15   // Secondary OK button pin (alternative)
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
// DAC8568 Configuration (8-Channel CV Output)
// ============================================================================
// Texas Instruments DAC8568: 16-bit, 8-channel DAC with SPI interface
// Provides 8 independent CV outputs (0-5V) for polyphonic or multi-parameter control

#define DAC_CS        16   // DAC chip select pin (/SYNC)
#define DAC_MOSI      26   // SPI1 MOSI (dedicated for DAC, avoids display conflicts)
#define DAC_SCK       27   // SPI1 SCK (dedicated for DAC)
#define DAC_RST       17   // DAC reset pin (optional - can tie to 5V if not used)
// DAC uses SPI1 bus to avoid conflicts with display/touch (which use SPI)

// DAC8568 Wiring:
//   DAC8568 VDD    -> Teensy 5V
//   DAC8568 VSS    -> Teensy GND
//   DAC8568 VREFIN -> 5V (external reference for 0-5V output range)
//   DAC8568 VREFOUT -> Not connected (or 100nF capacitor to GND if using internal ref)
//   DAC8568 SCLK   -> Teensy Pin 27 (SCK1 - dedicated SPI1 bus)
//   DAC8568 DIN    -> Teensy Pin 26 (MOSI1 - dedicated SPI1 bus)
//   DAC8568 SYNC   -> Teensy Pin 16 (DAC_CS - chip select)
//   DAC8568 LDAC   -> Teensy GND (tied low for immediate updates)
//   DAC8568 CLR    -> Teensy 5V (tied high, never clear)
//   DAC8568 DOUT   -> Not connected (or Pin 12 if daisy-chaining)
//   DAC8568 VOUTA-H -> CV outputs 0-7 (0-5V analog)
//
// Channel assignment:
//   Channel 0 (A): Poliquencer Step 1 CV / LFO primary output
//   Channel 1 (B): Poliquencer Step 2 CV
//   Channel 2 (C): Poliquencer Step 3 CV
//   Channel 3 (D): Poliquencer Step 4 CV
//   Channel 4 (E): Poliquencer Step 5 CV
//   Channel 5 (F): Poliquencer Step 6 CV
//   Channel 6 (G): Poliquencer Step 7 CV
//   Channel 7 (H): Poliquencer Step 8 CV / Gate output
//
// Note: DAC uses SPI1 (Pin 26/27) to avoid conflicts with display/touch (SPI0 Pin 11/13)
//       This completely isolates the DAC from display communication
// 
#define DAC_MAX_VALUE 65535     // 16-bit DAC (0-65535)
#define DAC_MAX_VOLTAGE 5.0f    // Maximum output voltage (with 5V reference)
#define GATE_HIGH_VOLTAGE 5.0f  // Gate high voltage
#define GATE_LOW_VOLTAGE 0.0f   // Gate low voltage

// DAC8568 channel definitions for easy reference
#define DAC_CH_STEP1    0  // Poliquencer step 1 / LFO output
#define DAC_CH_STEP2    1  // Poliquencer step 2
#define DAC_CH_STEP3    2  // Poliquencer step 3
#define DAC_CH_STEP4    3  // Poliquencer step 4
#define DAC_CH_STEP5    4  // Poliquencer step 5
#define DAC_CH_STEP6    5  // Poliquencer step 6
#define DAC_CH_STEP7    6  // Poliquencer step 7
#define DAC_CH_STEP8    7  // Poliquencer step 8 / Gate output

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
