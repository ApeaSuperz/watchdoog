#include <Arduino.h>
#include "constants.h"
#include "security/as_608_fingerprint.h"
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
        screen.setPowerSave(false);
        screen.u8g2.firstPage();
        do {
            screen.setFontSize(16);
            screen.drawCenter(("你坤吧谁？"));
        } while (screen.u8g2.nextPage());
        return true;
    }
    return false;
}

static void displayMenu(const char *addition = nullptr) {
    screen.setPowerSave(false);
    screen.u8g2.firstPage();
    do {
        screen.setFontSize(12);
        screen.draw(("A.添加指纹"), 0, 0);
        screen.drawEndHorizontal(("B.管理密码"), 0);
        screen.setFontSize(13);
        screen.drawCenterHorizontal(("欢迎使用"), (Screen::HEIGHT - 13 - 16) / 2);
        screen.setFontSize(16);
        screen.drawCenterHorizontal(("Watchdoog"), 13 + (Screen::HEIGHT - 13 - 16) / 2);

        if (addition != nullptr) {
            screen.setFontSize(12);
            screen.drawEndVerticalCenterHorizontal(addition);
        }
    } while (screen.u8g2.nextPage());
}

static const char *getHiddenPassword() {
    static char *password = nullptr;
    free(password);

    password = static_cast<char *>(malloc(keyboard.getInputLength() + 1));
    for (uint8_t i = 0; i < keyboard.getInputLength(); ++i) {
        password[i] = '*';
    }
    password[keyboard.getInputLength()] = '\0';

    return password;
}

static boolean requireAccess(const char *title, uint8_t timeoutInSeconds) {
    screen.u8g2.firstPage();
    do {
        screen.setFontSize(12);
        screen.drawCenterHorizontal(title, 0);
        screen.setFontSize(14);
        screen.drawCenterHorizontal(("验证已有指纹或密码"), (Screen::HEIGHT - 12 - 14) / 2 + 12);
    } while (screen.u8g2.nextPage());

    while (tick < timeoutInSeconds SECONDS) {
        tick++;
        keyboard.tick();

        if (finger.isFingerPressed()) {
            finger.setPowerSave(false);

            int32_t status = finger.verify(&screen);
            if (status > 0) return true;
        } else {
            finger.setPowerSave(true);
        }

        if (keyboard.hasNewInput()) {
            // if (Passwords::verify(keyboard.getInput())) return true;
            if (strcmp(keyboard.getInput(), SUPER_ADMIN_PASSWORD) == 0) return true;

            screen.u8g2.firstPage();
            do {
                screen.setFontSize(12);
                screen.drawCenterHorizontal(title, 0);
                screen.setFontSize(14);
                screen.drawCenterHorizontal(keyboard.getInput(), (Screen::HEIGHT - 12 - 14) / 2 + 12);
            } while (screen.u8g2.nextPage());
        }

        delay(TICK);
    }

    // 超时未验证
    return false;
}

static void addFingerprint() {
    if (requireAccess(("添加指纹"), 15)) {
        finger.enroll(finger.count() + 1, &screen);
    }
}

// static void managerPasswords() {
//     if (requireAccess(("管理密码"), 15)) {
//         screen.u8g2.firstPage();
//         do {
//             screen.setFontSize(14);
//             screen.draw(("1. 添加密码"), 0, 0);
//             screen.draw(("2. 删除密码"), 0, 14);
//             screen.draw(("3. 返回"), 0, 28);
//         } while (screen.u8g2.nextPage());
//
//         while (true) {
//             if (keyboard.hasNewInput()) {
//                 switch (keyboard.getInput()[0]) {
//                     case '1':
//                         Passwords::add(keyboard, &screen);
//                         return;
//                     case '2':
//                         Passwords::remove(keyboard, &screen);
//                         return;
//                     case '3':
//                         return;
//                     default:
//                         break;
//                 }
//                 keyboard.clear();
//             }
//             delay(TICK);
//         }
//     }
// }

void setup() {
    Serial.begin(9600);
    while (!Serial);  // For Yun/Leo/Micro/Zero/...
    delay(100);

    buzzer.setup();
    finger.setup();
    screen.setup();
    motor.setup();
    keyboard.setup();
    // Data::setup();
    // Passwords::setup();

    if (!finger.isConnected()) {
        screen.u8g2.firstPage();
        screen.setFontSize(14);
        do {
            screen.drawCenter(("找不到指纹模块 :("));
        } while (screen.u8g2.nextPage());
        buzzer.warning();
        while (true) delay(1);
    }

    displayMenu();
}

void loop() {
    tick++;
    keyboard.tick();

    const boolean isFingerPressed = finger.isFingerPressed();
    if (needScanFinger && isFingerPressed) {
        finger.setPowerSave(false);
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

            keyboard.clear();
        }
    } else if (!isFingerPressed) {
        finger.setPowerSave(true);
        needScanFinger = true;
    }

    if (keyboard.hasNewInput()) {
        tick = 0;

        if (screen.isPowerSave()) {
            keyboard.clear();
            screen.setPowerSave(false);
            displayMenu();
            return;
        } else {
            displayMenu(getHiddenPassword());
        }

        if (keyboard.getInputLength() == 1) {
            switch (keyboard.getInput()[0]) {
                case 'A':
                    keyboard.clear();
                    addFingerprint();
                    break;
                case 'B':
                    keyboard.clear();
                    // managerPasswords();
                    break;
                default:
                    break;
            }
        }

        // if (Passwords::verify(keyboard.getInput())) {
        //     keyboard.clear();
        //     buzzer.success();
        //     motor.rotate(180);
        //     rotsCount = 0;
        //     needResetMotor = true;
        // } else {
        //     screen.u8g2.firstPage();
        //     do {
        //         screen.setFontSize(13);
        //         screen.drawCenter(keyboard.getInput());
        //     } while (screen.u8g2.nextPage());
        // }
        if (strcmp(keyboard.getInput(), SUPER_ADMIN_PASSWORD) == 0) {
            keyboard.clear();
            buzzer.success();
            motor.rotate(180);
            rotsCount = 0;
            needResetMotor = true;
        }
    }

    if (needResetMotor && tick > 3 SECONDS) {
        motor.rotate(0);
        needResetMotor = false;
    }

    if (tick > 10 SECONDS) {
        screen.setPowerSave(true);
        keyboard.clear();
        tick = 0;
    }

    delay(TICK);
}
