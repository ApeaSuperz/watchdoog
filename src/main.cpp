#include <Arduino.h>
#include "constants.h"
#include "finger/as_608_fingerprint.h"
#include "buzzer/active_buzzer.h"
#include "display/ssd_1306_oled.h"

#if (defined(__AVR__) || defined(ESP8266)) && !defined(__AVR_ATmega2560__)
// For UNO and others without hardware serial, we must use software serial...
// pin #2 is IN from sensor (GREEN wire)
// pin #3 is OUT from arduino  (WHITE wire)
// Set up the serial port to use softwareserial..
SoftwareSerial fingerprint_serial(2, 3);

#else
// On Leonardo/M0/etc, others with hardware serial, use hardware serial!
// #0 is green wire, #1 is white
#define fingerprint_serial Serial1

#endif

uint8_t read_number();

static const unsigned int TICK = 100;

unsigned long long tick = 0;
boolean needScanFinger = true;

Finger finger = Finger(&fingerprint_serial, 4);
ActiveBuzzer buzzer = ActiveBuzzer(11);
Screen screen = Screen();

void setup() {
    Serial.begin(9600);
    while (!Serial);  // For Yun/Leo/Micro/Zero/...
    delay(100);

    buzzer.setup();
    finger.setup();
    screen.setup();

    screen.u8g2.firstPage();
    screen.u8g2.setFont(u8g2_font_watchdoog14_t);
    if (!finger.getCore().verifyPassword()) {
        do {
            screen.u8g2.setCursor((128 - 13 * 8) / 2, (64 - 13) / 2 + 8);
            screen.u8g2.print(F("找不到指纹模块 :("));
        } while (screen.u8g2.nextPage());
        buzzer.warning();
        while (true) delay(1);
    }
    do {
        screen.u8g2.setCursor((128 - 13 * 8) / 2, (64 - 13) / 2 + 8);
        screen.u8g2.print(F("已找到指纹模块 :)"));
    } while (screen.u8g2.nextPage());

    // Serial.println(F("Ready to enroll a fingerprint!"));
    // Serial.println(F("Please type in the ID # (from 1 to 127) you want to save this finger as..."));
    // uint8_t id = read_number();
    // if (id == 0) {  // ID #0 not allowed, try again!
    //     return;
    // }
    // Serial.print(F("Enrolling ID #"));
    // Serial.println(id);
    // while (finger.enroll(id) != FINGERPRINT_OK);
    // buzzer.success();
    screen.u8g2.firstPage();
    do {
        screen.u8g2.setFont(u8g2_font_watchdoog13_t);
        screen.drawCenterHorizontal(F("欢迎使用"), 13 + (64 - 13 - 16) / 2);
        screen.u8g2.setFont(u8g2_font_watchdoog16_t);
        screen.drawCenterHorizontal(F("Watchdoog"), 13 + 16 + (64 - 13 - 16) / 2);
    } while (screen.u8g2.nextPage());

    // finger.enroll(1, &screen);
}

uint8_t read_number() {
    uint8_t num = 0;

    while (num == 0) {
        while (!Serial.available());
        num = Serial.parseInt();
    }
    return num;
}

void loop() {
    tick++;

    const boolean isFingerPressed = finger.isFingerPressed();
    if (needScanFinger && isFingerPressed) {
        finger.getCore().LEDcontrol(true);
        int32_t status = finger.verify(&screen);
        if (status <= 0 && status != finger.NO_FINGER) {
            buzzer.warning();

            tick = 0;
            needScanFinger = false;
        } else if (status > 0) {
            buzzer.success();

            tick = 0;
            needScanFinger = false;
        }
    } else if (!isFingerPressed) {
        finger.getCore().LEDcontrol(false);
        needScanFinger = true;
    }

    if (tick > 100) {
        screen.u8g2.setPowerSave(true);
        tick = 0;
    }

    delay(TICK);
}
