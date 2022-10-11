#ifndef WATCHDOOG_SSD_1306_LED_H
#define WATCHDOOG_SSD_1306_LED_H

#include <Arduino.h>
#include <U8g2lib.h>

#ifdef U8X8_HAVE_HW_SPI

#include <SPI.h>

#endif

#ifdef U8X8_HAVE_HW_I2C

#include <Wire.h>

#endif

#include "watchdoog_fonts.h"

class Screen {
public:
    static const uint8_t WIDTH = 128;
    static const uint8_t HEIGHT = 64;

    Screen();

    void setup();

    uint8_t computeStringWidth(const __FlashStringHelper *str);

    uint8_t computeStringWidth(const char *str);

    void draw(const __FlashStringHelper *text, uint8_t x, uint8_t y);

    void draw(const char *text, uint8_t x, uint8_t y);

    void drawCenterHorizontal(const __FlashStringHelper *text, uint8_t y);

    void drawCenterHorizontal(const char *text, uint8_t y);

    void drawEndHorizontal(const __FlashStringHelper *text, uint8_t y);

    void drawEndHorizontal(const char *text, uint8_t y);

    void setFontSize(uint8_t size);

    U8G2_SSD1306_128X64_NONAME_2_SW_I2C u8g2;
};


#endif //WATCHDOOG_SSD_1306_LED_H
