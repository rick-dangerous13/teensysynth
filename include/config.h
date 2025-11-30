/**
 * TeensySynth Configuration
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
// Potentiometer Configuration
// ============================================================================
#define POT_SCROLL    A0   // Scroll potentiometer pin

// Potentiometer ADC settings (Teensy 4.1 has 10-bit ADC by default)
#define POT_MIN       0
#define POT_MAX       1023
#define POT_DEADZONE  20   // Ignore small changes

// ============================================================================
// Script Manager Configuration
// ============================================================================
#define MAX_SCRIPTS   4    // Maximum concurrent scripts

// ============================================================================
// UI Configuration - Norns-style aesthetics
// ============================================================================
// Colors (16-bit RGB565)
#define COLOR_BG           0x0000  // Black background
#define COLOR_FG           0xFFFF  // White foreground
#define COLOR_ACCENT       0x07FF  // Cyan accent
#define COLOR_DIM          0x7BEF  // Gray for inactive elements
#define COLOR_HIGHLIGHT    0xFFE0  // Yellow highlight

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
#define DISPLAY_UPDATE_INTERVAL 50
#define SCRIPT_UPDATE_INTERVAL  10

#endif // CONFIG_H
