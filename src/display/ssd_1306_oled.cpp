#include "ssd_1306_oled.h"

Screen::Screen() : u8g2(U8G2_R0, SCL, SDA) {}

void Screen::setup() {
    u8g2.begin();
    u8g2.enableUTF8Print();

    setFontSize(0);
}

uint8_t Screen::computeStringWidth(const __FlashStringHelper *str) {
    uint8_t stringLength = strlen_P(reinterpret_cast<const char *>(str));
    auto *string = reinterpret_cast<char *>(malloc((stringLength + 1) * sizeof(char)));
    for (int i = 0; i < stringLength; i++) {
        string[i] = pgm_read_byte_near(reinterpret_cast<int>(str) + i);
    }
    string[stringLength] = '\0';
    uint8_t result = computeStringWidth(string);
    free(string);
    return result;
}

uint8_t Screen::computeStringWidth(const char *str) {
    return u8g2.getUTF8Width(str);
}

void Screen::draw(const __FlashStringHelper *text, uint8_t x, uint8_t y) {
    u8g2.setCursor(x, getSuitY(y));
    u8g2.print(text);
}

void Screen::draw(const char *text, uint8_t x, uint8_t y) {
    u8g2.drawUTF8(x, getSuitY(y), text);
}

void Screen::drawCenter(const __FlashStringHelper *text) {
    uint8_t stringLength = strlen_P(reinterpret_cast<const char *>(text));
    auto *string = reinterpret_cast<char *>(malloc((stringLength + 1) * sizeof(char)));
    for (int i = 0; i < stringLength; i++) {
        string[i] = pgm_read_byte_near(reinterpret_cast<int>(text) + i);
    }
    string[stringLength] = '\0';
    drawCenter(string);
    free(string);
}

void Screen::drawCenter(const char *text) {
    const uint8_t x = (WIDTH - computeStringWidth(text)) / 2;
    u8g2.drawUTF8(x, (HEIGHT - fontSize) / 2 + fontSize, text);
}

void Screen::drawCenterHorizontal(const __FlashStringHelper *text, uint8_t y) {
    uint8_t stringLength = strlen_P(reinterpret_cast<const char *>(text));
    auto *string = reinterpret_cast<char *>(malloc((stringLength + 1) * sizeof(char)));
    for (int i = 0; i < stringLength; i++) {
        string[i] = pgm_read_byte_near(reinterpret_cast<int>(text) + i);
    }
    string[stringLength] = '\0';
    drawCenterHorizontal(string, y);
    free(string);
}

void Screen::drawCenterHorizontal(const char *text, uint8_t y) {
    const uint8_t x = (WIDTH - computeStringWidth(text)) / 2;
    u8g2.drawUTF8(x, getSuitY(y), text);
}

void Screen::drawEndHorizontal(const __FlashStringHelper *text, uint8_t y) {
    uint8_t stringLength = strlen_P(reinterpret_cast<const char *>(text));
    auto *string = reinterpret_cast<char *>(malloc((stringLength + 1) * sizeof(char)));
    for (int i = 0; i < stringLength; i++) {
        string[i] = pgm_read_byte_near(reinterpret_cast<int>(text) + i);
    }
    string[stringLength] = '\0';
    drawEndHorizontal(string, y);
    free(string);
}

void Screen::drawEndHorizontal(const char *text, uint8_t y) {
    const uint8_t x = WIDTH - computeStringWidth(text);
    u8g2.drawUTF8(x, getSuitY(y), text);
}

void Screen::drawEndVerticalCenterHorizontal(const __FlashStringHelper *text, uint8_t x) {
    uint8_t stringLength = strlen_P(reinterpret_cast<const char *>(text));
    auto *string = reinterpret_cast<char *>(malloc((stringLength + 1) * sizeof(char)));
    for (int i = 0; i < stringLength; i++) {
        string[i] = pgm_read_byte_near(reinterpret_cast<int>(text) + i);
    }
    string[stringLength] = '\0';
    drawEndVerticalCenterHorizontal(string, x);
    free(string);
}

void Screen::drawEndVerticalCenterHorizontal(const char *text, uint8_t x) {
    const uint8_t y = HEIGHT - computeStringWidth(text);
    u8g2.drawUTF8(x, getSuitY(y), text);
}

void Screen::setFontSize(uint8_t size) {
    static const uint8_t DEFAULT_FONT_SIZE = 14;

    switch (size) {
        case 12:
            u8g2.setFont(u8g2_font_watchdoog12_t);
            break;
        case 13:
            u8g2.setFont(u8g2_font_watchdoog13_t);
            break;
        case 15:
            u8g2.setFont(u8g2_font_watchdoog15_t);
            break;
        case 16:
            u8g2.setFont(u8g2_font_watchdoog16_t);
            break;
        case DEFAULT_FONT_SIZE:
        default:
            u8g2.setFont(u8g2_font_watchdoog14_t);
            break;
    }

    fontSize = size >= 12 && size <= 16 ? size : DEFAULT_FONT_SIZE;
}

uint8_t Screen::getSuitY(uint8_t y) const {
    return y + fontSize >= HEIGHT ? HEIGHT - 1 : y + fontSize;
}

void Screen::setPowerSave(boolean state) {
    u8g2.setPowerSave(state);
    isPowerSaving = state;
}

boolean Screen::isPowerSave() const {
    return isPowerSaving;
}
