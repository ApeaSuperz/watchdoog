#include "as_608_fingerprint.h"

Finger::Finger(SoftwareSerial *serial, int8_t touchPin, uint32_t password) :
        touchPin(touchPin),
        finger(Adafruit_Fingerprint(serial, password)) {}

void Finger::setup() {
    // set the data rate for the sensor serial port
    finger.begin(57600);

    if (touchPin != 0) {
        pinMode(touchPin, INPUT);
    }
}

boolean Finger::isConnected() {
    return finger.verifyPassword();
}

boolean Finger::isFingerPressed() const {
    if (touchPin == -1) {
        return true;
    }
    return digitalRead(touchPin) == HIGH;
}

Adafruit_Fingerprint Finger::getCore() {
    return finger;
}

// TODO: display on LED
int8_t Finger::enroll(uint8_t id, Screen *screen) {
    // Serial.print(F("Waiting for valid finger to enroll as #"));
    uint8_t fingerIdX = 0;
    if (screen != nullptr) {
        screen->u8g2.firstPage();
        do {
            screen->setFontSize(12);
            screen->draw(F("注册指纹#"), 0, 12);
            fingerIdX = screen->computeStringWidth(F("注册指纹#")) + 2;
            screen->u8g2.setCursor(fingerIdX, 12);
            screen->u8g2.print(id);
            screen->setFontSize(14);
            screen->drawCenterHorizontal(F("请按下手指"), 14 + (Screen::HEIGHT - 12 - 14) / 2);
        } while (screen->u8g2.nextPage());
    }
    // Serial.println(id);

    int status = -1;
    while (status != FINGERPRINT_OK) {
        status = finger.getImage();
        switch (status) {
            case FINGERPRINT_OK:
                // Serial.println(F("Image taken"));
                if (screen != nullptr) {
                    screen->u8g2.firstPage();
                    do {
                        screen->setFontSize(12);
                        screen->draw(F("注册指纹#"), 0, 12);
                        screen->u8g2.setCursor(fingerIdX, 12);
                        screen->u8g2.print(id);
                        screen->setFontSize(14);
                        screen->drawCenterHorizontal(F("成像中..."), 14 + (Screen::HEIGHT - 12 - 14) / 2);
                    } while (screen->u8g2.nextPage());
                }
                break;
            case FINGERPRINT_NOFINGER:
                if (screen != nullptr) {
                    screen->u8g2.firstPage();
                    do {
                        screen->setFontSize(12);
                        screen->draw(F("注册指纹#"), 0, 12);
                        screen->u8g2.setCursor(fingerIdX, 12);
                        screen->u8g2.print(id);
                        screen->setFontSize(14);
                        screen->drawCenterHorizontal(F("请按下手指"), 14 + (Screen::HEIGHT - 12 - 14) / 2);
                        screen->setFontSize(12);
                        screen->drawEndHorizontal(F("正在等待手指..."), Screen::HEIGHT - 12);
                    } while (screen->u8g2.nextPage());
                }
                break;
            case FINGERPRINT_PACKETRECIEVEERR:
                // Serial.println(F("Communication error"));
                if (screen != nullptr) {
                    screen->u8g2.firstPage();
                    do {
                        screen->setFontSize(12);
                        screen->drawCenterHorizontal(F("ERROR"), 12 + (Screen::HEIGHT - 12 - 16) / 2);
                        screen->setFontSize(16);
                        screen->drawCenterHorizontal(F("通信错误"), 12 + 16 + (Screen::HEIGHT - 12 - 16) / 2);
                    } while (screen->u8g2.nextPage());
                }
                break;
            case FINGERPRINT_IMAGEFAIL:
                // Serial.println(F("Imaging error"));
                if (screen != nullptr) {
                    screen->u8g2.firstPage();
                    do {
                        screen->setFontSize(12);
                        screen->drawCenterHorizontal(F("ERROR"), 12 + (Screen::HEIGHT - 12 - 16) / 2);
                        screen->setFontSize(16);
                        screen->drawCenterHorizontal(F("无法成像"), 12 + 16 + (Screen::HEIGHT - 12 - 16) / 2);
                    } while (screen->u8g2.nextPage());
                }
                break;
            default:
                // Serial.println(F("Unknown error"));
                if (screen != nullptr) {
                    screen->u8g2.firstPage();
                    do {
                        screen->setFontSize(12);
                        screen->drawCenterHorizontal(F("ERROR"), 12 + (Screen::HEIGHT - 12 - 16) / 2);
                        screen->setFontSize(16);
                        screen->drawCenterHorizontal(F("未知错误"), 12 + 16 + (Screen::HEIGHT - 12 - 16) / 2);
                    } while (screen->u8g2.nextPage());
                }
                break;
        }
    }

    status = finger.image2Tz(1);
    switch (status) {
        case FINGERPRINT_OK:
            // Serial.println(F("Image converted"));
            break;
        case FINGERPRINT_IMAGEMESS:
            // Serial.println(F("Image too messy"));
            return IMAGE_MESS;
        case FINGERPRINT_PACKETRECIEVEERR:
            // Serial.println(F("Communication error"));
            return PACKET_RECEIVE_ERROR;
        case FINGERPRINT_FEATUREFAIL:
            // Serial.println(F("Could not find fingerprint features"));
            return FEATURE_FAIL;
        case FINGERPRINT_INVALIDIMAGE:
            // Serial.println(F("Could not find fingerprint features"));
            return INVALID_IMAGE;
        default:
            // Serial.println(F("Unknown error"));
            return UNKNOWN_ERROR;
    }

    // Serial.println(F("Remove your finger"));
    delay(2000);
    status = 0;
    while (status != FINGERPRINT_NOFINGER) {
        status = finger.getImage();
    }

    status = -1;
    // Serial.print(F("Place same finger again"));
    while (status != FINGERPRINT_OK) {
        status = finger.getImage();
        switch (status) {
            case FINGERPRINT_OK:
                // Serial.println(F("Image taken"));
                break;
            case FINGERPRINT_NOFINGER:
                // Serial.print(F("."));
                break;
            case FINGERPRINT_PACKETRECIEVEERR:
                // Serial.println(F("Communication error"));
                break;
            case FINGERPRINT_IMAGEFAIL:
                // Serial.println(F("Imaging error"));
                break;
            default:
                // Serial.println(F("Unknown error"));
                break;
        }
    }

    status = finger.image2Tz(2);
    switch (status) {
        case FINGERPRINT_OK:
            // Serial.println(F("Image converted"));
            break;
        case FINGERPRINT_IMAGEMESS:
            // Serial.println(F("Image too messy"));
            return IMAGE_MESS;
        case FINGERPRINT_PACKETRECIEVEERR:
            // Serial.println(F("Communication error"));
            return PACKET_RECEIVE_ERROR;
        case FINGERPRINT_FEATUREFAIL:
            // Serial.println(F("Could not find fingerprint features"));
            return FEATURE_FAIL;
        case FINGERPRINT_INVALIDIMAGE:
            // Serial.println(F("Could not find fingerprint features"));
            return INVALID_IMAGE;
        default:
            // Serial.println(F("Unknown error"));
            return UNKNOWN_ERROR;
    }

    // Serial.print(F("Creating model for #"));
    Serial.println(id);
    status = finger.createModel();
    switch (status) {
        case FINGERPRINT_OK:
            // Serial.println(F("Prints matched!"));
            break;
        case FINGERPRINT_PACKETRECIEVEERR:
            // Serial.println(F("Communication error"));
            return PACKET_RECEIVE_ERROR;
        case FINGERPRINT_ENROLLMISMATCH:
            // Serial.println(F("Fingerprints did not match"));
            return ENROLL_MISMATCH;
        default:
            // Serial.println(F("Unknown error"));
            return UNKNOWN_ERROR;
    }

    status = finger.storeModel(id);
    switch (status) {
        case FINGERPRINT_OK:
            // Serial.println(F("Stored!"));
            break;
        case FINGERPRINT_PACKETRECIEVEERR:
            // Serial.println(F("Communication error"));
            return PACKET_RECEIVE_ERROR;
        case FINGERPRINT_BADLOCATION:
            // Serial.println(F("Could not store in that location"));
            return BAD_LOCATION;
        case FINGERPRINT_FLASHERR:
            // Serial.println(F("Error writing to flash"));
            return FLASH_ERROR;
        default:
            // Serial.println(F("Unknown error"));
            return UNKNOWN_ERROR;
    }

    return OK;
}

int32_t Finger::verify() {
    uint8_t status = finger.getImage();
    switch (status) {
        case FINGERPRINT_OK:
            // Serial.println(F("Image taken"));
            break;
        case FINGERPRINT_NOFINGER:
            return NO_FINGER;
        case FINGERPRINT_PACKETRECIEVEERR:
            // Serial.println(F("Communication error"));
            return PACKET_RECEIVE_ERROR;
        case FINGERPRINT_IMAGEFAIL:
            // Serial.println(F("Imaging error"));
            return IMAGE_FAIL;
        default:
            // Serial.println(F("Unknown error"));
            return UNKNOWN_ERROR;
    }

    status = finger.image2Tz();
    switch (status) {
        case FINGERPRINT_OK:
            // Serial.println(F("Image converted"));
            break;
        case FINGERPRINT_IMAGEMESS:
            // Serial.println(F("Image too messy"));
            return IMAGE_MESS;
        case FINGERPRINT_PACKETRECIEVEERR:
            // Serial.println(F("Communication error"));
            return PACKET_RECEIVE_ERROR;
        case FINGERPRINT_FEATUREFAIL:
            // Serial.println(F("Could not find fingerprint features"));
            return FEATURE_FAIL;
        case FINGERPRINT_INVALIDIMAGE:
            // Serial.println(F("Could not find fingerprint features"));
            return INVALID_IMAGE;
        default:
            // Serial.println(F("Unknown error"));
            return UNKNOWN_ERROR;
    }

    status = finger.fingerSearch();
    switch (status) {
        case FINGERPRINT_OK:
            // Serial.println(F("Found a print match!"));
            break;
        case FINGERPRINT_PACKETRECIEVEERR:
            // Serial.println(F("Communication error"));
            return PACKET_RECEIVE_ERROR;
        case FINGERPRINT_NOTFOUND:
            // Serial.println(F("Did not find a match"));
            return NOT_FOUND;
        default:
            // Serial.println(F("Unknown error"));
            return UNKNOWN_ERROR;
    }

    // Serial.print(F("Found ID #"));
    // Serial.println(finger.fingerID);
    // Serial.print(F(" with confidence of "));
    // Serial.println(finger.confidence);

    return finger.fingerID;
}

// TODO: display on LED
uint8_t Finger::remove(uint8_t id) {
    uint8_t status = finger.deleteModel(id);

    switch (status) {
        case FINGERPRINT_OK:
            // Serial.println(F("Deleted!"));
            break;
        case FINGERPRINT_PACKETRECIEVEERR:
            // Serial.println(F("Communication error"));
            break;
        case FINGERPRINT_BADLOCATION:
            // Serial.println(F("Could not delete in that location"));
            break;
        case FINGERPRINT_FLASHERR:
            // Serial.println(F("Error writing to flash"));
            break;
        default:
            // Serial.print(F("Unknown error: 0x"));
            // Serial.println(status, HEX);
            break;
    }

    return status;
}

uint8_t Finger::count() {
    uint8_t status = finger.getTemplateCount();
    if (status == FINGERPRINT_OK) return finger.templateCount;
    return 0;
}
