/**
 * Display Driver for ILI9341
 * 
 * Wraps the ILI9341_t3 library with Norns-style graphics primitives
 */

#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include <ILI9341_t3.h>
#include "config.h"

class Display {
public:
    Display();
    
    void begin();
    void clear();
    void update();
    
    // Text drawing
    void drawText(int16_t x, int16_t y, const char* text, uint16_t color = COLOR_FG, uint8_t size = FONT_MEDIUM);
    void drawTextCentered(int16_t y, const char* text, uint16_t color = COLOR_FG, uint8_t size = FONT_MEDIUM);
    
    // Primitive drawing
    void drawPixel(int16_t x, int16_t y, uint16_t color);
    void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
    void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
    void drawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color);
    void fillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color);
    void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
    void drawCircle(int16_t x, int16_t y, int16_t r, uint16_t color);
    void fillCircle(int16_t x, int16_t y, int16_t r, uint16_t color);
    
    // Norns-style UI elements
    void drawMenuBox(int16_t x, int16_t y, int16_t w, int16_t h, bool selected = false);
    void drawProgressBar(int16_t x, int16_t y, int16_t w, int16_t h, float progress);
    void drawWaveform(int16_t x, int16_t y, int16_t w, int16_t h, const int16_t* data, int16_t len);
    void drawScrollIndicator(int16_t y, int16_t totalItems, int16_t visibleItems, int16_t currentItem);
    
    // Text measurement
    void getTextBounds(const char* text, int16_t x, int16_t y, int16_t* x1, int16_t* y1, uint16_t* w, uint16_t* h, uint8_t size = FONT_MEDIUM);
    
    // Screen regions for 4-script multitasking view
    void drawQuadrantDividers();
    void setClipRegion(uint8_t quadrant);
    void clearClipRegion();

private:
    ILI9341_t3 tft;
    
    // Clipping region for quadrant rendering
    int16_t clipX, clipY, clipW, clipH;
    bool clippingEnabled;
};

#endif // DISPLAY_H
