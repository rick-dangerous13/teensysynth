/**
 * Display Driver Implementation
 * 
 * ILI9341 display driver with Norns-style graphics
 */

#include "display.h"
#include <string.h>

Display::Display() 
    : tft(TFT_CS, TFT_DC, TFT_RST, TFT_MOSI, TFT_SCLK, TFT_MISO),
      clipX(0), clipY(0), clipW(SCREEN_WIDTH), clipH(SCREEN_HEIGHT),
      clippingEnabled(false) {
}

void Display::begin() {
    tft.begin();
    tft.setRotation(3);  // Landscape mode (flipped)
    clear();
}

void Display::clear() {
    tft.fillScreen(COLOR_BG);
}

void Display::update() {
    // ILI9341_t3 doesn't need explicit update - writes are immediate
    // This method is here for potential double-buffering in the future
}

void Display::drawText(int16_t x, int16_t y, const char* text, uint16_t color, uint8_t size) {
    tft.setTextColor(color);
    tft.setTextSize(size);
    tft.setCursor(x, y);
    tft.print(text);
}

void Display::drawTextCentered(int16_t y, const char* text, uint16_t color, uint8_t size) {
    // Calculate text width manually (ILI9341_t3 uses 6x8 font)
    int16_t charWidth = 6 * size;
    int16_t textWidth = strlen(text) * charWidth;
    int16_t x = (SCREEN_WIDTH - textWidth) / 2;
    drawText(x, y, text, color, size);
}

void Display::drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    tft.drawRect(x, y, w, h, color);
}

void Display::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    tft.fillRect(x, y, w, h, color);
}

void Display::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) {
    tft.drawLine(x0, y0, x1, y1, color);
}

void Display::drawCircle(int16_t x, int16_t y, int16_t r, uint16_t color) {
    tft.drawCircle(x, y, r, color);
}

void Display::fillCircle(int16_t x, int16_t y, int16_t r, uint16_t color) {
    tft.fillCircle(x, y, r, color);
}

void Display::drawMenuBox(int16_t x, int16_t y, int16_t w, int16_t h, bool selected) {
    if (selected) {
        // Filled box with inverted colors for selection
        fillRect(x, y, w, h, COLOR_FG);
    } else {
        // Outlined box
        drawRect(x, y, w, h, COLOR_DIM);
    }
}

void Display::drawProgressBar(int16_t x, int16_t y, int16_t w, int16_t h, float progress) {
    // Clamp progress to 0-1
    if (progress < 0.0f) progress = 0.0f;
    if (progress > 1.0f) progress = 1.0f;
    
    // Draw outline
    drawRect(x, y, w, h, COLOR_FG);
    
    // Draw filled portion
    int16_t fillW = (int16_t)((w - 2) * progress);
    if (fillW > 0) {
        fillRect(x + 1, y + 1, fillW, h - 2, COLOR_ACCENT);
    }
}

void Display::drawWaveform(int16_t x, int16_t y, int16_t w, int16_t h, const int16_t* data, int16_t len) {
    if (len < 2 || data == nullptr) return;
    
    int16_t centerY = y + h / 2;
    
    for (int16_t i = 0; i < len - 1; i++) {
        int16_t x0 = x + (i * w) / (len - 1);
        int16_t x1 = x + ((i + 1) * w) / (len - 1);
        int16_t y0 = centerY - (data[i] * h) / (2 * 32768);
        int16_t y1 = centerY - (data[i + 1] * h) / (2 * 32768);
        drawLine(x0, y0, x1, y1, COLOR_ACCENT);
    }
}

void Display::drawScrollIndicator(int16_t y, int16_t totalItems, int16_t visibleItems, int16_t currentItem) {
    if (totalItems <= visibleItems) return;
    
    int16_t scrollbarH = SCREEN_HEIGHT - 2 * MARGIN;
    int16_t thumbH = (scrollbarH * visibleItems) / totalItems;
    if (thumbH < 10) thumbH = 10;
    
    int16_t thumbY = y + (scrollbarH - thumbH) * currentItem / (totalItems - 1);
    
    // Draw track
    drawLine(SCREEN_WIDTH - 5, y, SCREEN_WIDTH - 5, y + scrollbarH, COLOR_DIM);
    
    // Draw thumb
    fillRect(SCREEN_WIDTH - 7, thumbY, 5, thumbH, COLOR_FG);
}

void Display::drawQuadrantDividers() {
    // Draw cross dividing screen into 4 quadrants
    drawLine(SCREEN_WIDTH / 2, 0, SCREEN_WIDTH / 2, SCREEN_HEIGHT, COLOR_DIM);
    drawLine(0, SCREEN_HEIGHT / 2, SCREEN_WIDTH, SCREEN_HEIGHT / 2, COLOR_DIM);
}

void Display::setClipRegion(uint8_t quadrant) {
    clippingEnabled = true;
    
    int16_t halfW = SCREEN_WIDTH / 2;
    int16_t halfH = SCREEN_HEIGHT / 2;
    
    switch (quadrant) {
        case 0: // Top-left
            clipX = 0; clipY = 0;
            break;
        case 1: // Top-right
            clipX = halfW; clipY = 0;
            break;
        case 2: // Bottom-left
            clipX = 0; clipY = halfH;
            break;
        case 3: // Bottom-right
            clipX = halfW; clipY = halfH;
            break;
    }
    clipW = halfW;
    clipH = halfH;
}

void Display::clearClipRegion() {
    clippingEnabled = false;
    clipX = 0;
    clipY = 0;
    clipW = SCREEN_WIDTH;
    clipH = SCREEN_HEIGHT;
}

void Display::getTextBounds(const char* text, int16_t x, int16_t y, int16_t* x1, int16_t* y1, uint16_t* w, uint16_t* h, uint8_t size) {
    // Manual text bounds calculation for ILI9341_t3 (uses 6x8 font)
    *x1 = x;
    *y1 = y;
    *w = strlen(text) * 6 * size;
    *h = 8 * size;
}
