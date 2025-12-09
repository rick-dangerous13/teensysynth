/**
 * TFT_eSPI User Setup for ILI9488 IPS on Teensy 4.1
 * 
 * IMPORTANT: This display is IPS (In-Plane Switching), not TN!
 * Requires IPS-specific initialization for correct colors
 */

// ============================================================================
// User defined pins
// ============================================================================
#define TFT_MOSI 11
#define TFT_SCLK 13
#define TFT_CS   10
#define TFT_DC   9
#define TFT_RST  8
#define TFT_MISO 12

// ============================================================================
// SPI & Display Setup
// ============================================================================
#define ILI9488_DRIVER      // ILI9488 display driver - IPS version
#define TFT_WIDTH  320
#define TFT_HEIGHT 480

// Color order - IPS displays often have different color orders
#define TFT_RGB_ORDER TFT_BGR  // Try BGR for IPS

// Interface
#define TFT_SPI_READ_FREQUENCY 6000000   // Read SPI speed
#define TFT_SPI_FREQUENCY 30000000       // Write SPI speed

// For ESP32 S3 and other boards - no PSRAM needed for this size
#define SPI_FREQUENCY 30000000
#define SPI_READ_FREQUENCY 6000000

// Teensy specific
#define SUPPORT_TRANSACTIONS

// ============================================================================
// Font selection - Use GLCD font (built-in, minimal size)
// ============================================================================
// Disable all font loading to use built-in GFX font
#undef LOAD_GLCD
#undef LOAD_FONT2
#undef LOAD_FONT4
#undef LOAD_FONT6
#undef LOAD_FONT7
#undef LOAD_FONT8
#undef LOAD_GFXFF

// ============================================================================
// Other options
// ============================================================================
#define SPI_TOUCH_FREQUENCY  2500000     // Touch SPI speed
