#ifndef WATCHDOOG_WATCHDOOG_FONTS_H
#define WATCHDOOG_WATCHDOOG_FONTS_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef U8G2_USE_LARGE_FONTS
#define U8G2_USE_LARGE_FONTS
#endif

#ifndef U8X8_FONT_SECTION

#ifdef __GNUC__
#  define U8X8_SECTION(name) __attribute__ ((section (name)))
#else
#  define U8X8_SECTION(name)
#endif

#if defined(__GNUC__) && defined(__AVR__)
#  define U8X8_FONT_SECTION(name) U8X8_SECTION(".progmem." name)
#endif

#if defined(ESP8266)
#  define U8X8_FONT_SECTION(name) __attribute__((section(".text." name)))
#endif

#ifndef U8X8_FONT_SECTION
#  define U8X8_FONT_SECTION(name)
#endif

#endif

#ifndef U8G2_FONT_SECTION
#define U8G2_FONT_SECTION(name) U8X8_FONT_SECTION(name)
#endif

extern const uint8_t u8g2_font_watchdoog12_t[] U8G2_FONT_SECTION("u8g2_font_watchdoog12_t");
extern const uint8_t u8g2_font_watchdoog13_t[] U8G2_FONT_SECTION("u8g2_font_watchdoog13_t");
extern const uint8_t u8g2_font_watchdoog14_t[] U8G2_FONT_SECTION("u8g2_font_watchdoog14_t");
extern const uint8_t u8g2_font_watchdoog15_t[] U8G2_FONT_SECTION("u8g2_font_watchdoog15_t");
extern const uint8_t u8g2_font_watchdoog16_t[] U8G2_FONT_SECTION("u8g2_font_watchdoog16_t");

#ifdef __cplusplus
}
#endif

#endif //WATCHDOOG_WATCHDOOG_FONTS_H
