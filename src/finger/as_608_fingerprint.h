#ifndef WATCHDOOG_AS_608_FINGERPRINT_H
#define WATCHDOOG_AS_608_FINGERPRINT_H

#include <Arduino.h>
#include <Adafruit_Fingerprint.h>
#include "display/ssd_1306_oled.h"

class Finger {
public:
    explicit Finger(SoftwareSerial *serial, int8_t touchPin = -1, uint32_t password = 0x0);

    void setup();

    boolean isConnected();

    boolean isFingerPressed() const;

    Adafruit_Fingerprint getCore();

    int8_t enroll(uint8_t id, Screen *screen);

    int32_t verify();

    uint8_t remove(uint8_t id);

    uint8_t count();

    enum STATUS {
        OK = 0,
        NO_FINGER = -1,
        PACKET_RECEIVE_ERROR = -2,
        IMAGE_FAIL = -3,
        IMAGE_MESS = -4,
        FEATURE_FAIL = -5,
        INVALID_IMAGE = -6,
        TEMPLATE_FAIL = -7,
        MATCH_FAIL = -8,
        NOT_FOUND = -9,
        COMMUNICATION_ERROR = -10,
        TIMEOUT = -11,
        ENROLL_MISMATCH = -12,
        BAD_LOCATION = -13,
        FLASH_ERROR = -14,
        UNKNOWN_ERROR = -15,
    };

private:
    int8_t touchPin;
    Adafruit_Fingerprint finger;
};


#endif //WATCHDOOG_AS_608_FINGERPRINT_H
