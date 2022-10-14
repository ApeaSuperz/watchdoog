#include <Arduino.h>
#include "constants.h"
#include "finger/as_608_fingerprint.h"
#include "buzzer/active_buzzer.h"
#include "display/ssd_1306_oled.h"
#include "motor/sg_90_servo_motor.h"
#include "io/keyboard_4_x_4.h"

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

#define SECONDS * 1000 / TICK

static const unsigned int TICK = 50;

unsigned long long tick = 0;
uint8_t rotsCount = 0;
boolean needScanFinger = true;
boolean needResetMotor = false;

Finger finger = Finger(&fingerprint_serial, 4);
ActiveBuzzer buzzer = ActiveBuzzer(11);
Screen screen = Screen();
ServoMotor motor = ServoMotor(9);
Keyboard keyboard = Keyboard(new uint8_t[4]{6, 5, PIN_A0, PIN_A1}, new uint8_t[4]{12, 10, 8, 7});

static boolean triggerEgg() {
    if (rotsCount >= 3) {
        screen.u8g2.setPowerSave(false);
        screen.u8g2.firstPage();
        do {
            screen.setFontSize(16);
            screen.drawCenter(F("你坤吧谁？"));
        } while (screen.u8g2.nextPage());
        return true;
    }
    return false;
}

void setup() {
    Serial.begin(9600);
    while (!Serial);  // For Yun/Leo/Micro/Zero/...
    delay(100);

    buzzer.setup();
    finger.setup();
    screen.setup();
    motor.setup();
    keyboard.setup();

    screen.u8g2.firstPage();
    screen.setFontSize(14);
    if (!finger.isConnected()) {
        do {
            screen.drawCenter(F("找不到指纹模块 :("));
        } while (screen.u8g2.nextPage());
        buzzer.warning();
        while (true) delay(1);
    }

    screen.u8g2.firstPage();
    do {
        screen.setFontSize(13);
        screen.drawCenterHorizontal(F("欢迎使用"), (Screen::HEIGHT - 13 - 16) / 2);
        screen.setFontSize(16);
        screen.drawCenterHorizontal(F("Watchdoog"), 13 + (Screen::HEIGHT - 13 - 16) / 2);
    } while (screen.u8g2.nextPage());
}

void loop() {
    tick++;
    keyboard.tick();

    const boolean isFingerPressed = finger.isFingerPressed();
    if (needScanFinger && isFingerPressed) {
        finger.getCore().LEDcontrol(true);
        int32_t status = finger.verify(&screen);
        if (status <= 0 && status != finger.NO_FINGER) {
            buzzer.warning();

            tick = 0;
            rotsCount++;
            needScanFinger = false;

            triggerEgg();
        } else if (status > 0) {
            motor.rotate(180);
            buzzer.success();

            tick = 0;
            rotsCount = 0;
            needScanFinger = false;
            needResetMotor = true;
        }
    } else if (!isFingerPressed) {
        finger.getCore().LEDcontrol(false);
        needScanFinger = true;
    }

    if (keyboard.hasNewInput()) {
        screen.u8g2.setPowerSave(false);
        screen.u8g2.firstPage();
        do {
            screen.setFontSize(13);
            screen.drawCenter(keyboard.getInput());
        } while (screen.u8g2.nextPage());
        tick = 0;
    }

    if (needResetMotor && tick > 3 SECONDS) {
        motor.rotate(0);
        needResetMotor = false;
    }

    if (tick > 10 SECONDS) {
        screen.u8g2.setPowerSave(true);
        keyboard.clear();
        tick = 0;
    }

    delay(TICK);
}
