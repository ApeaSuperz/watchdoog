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

/**
 * 获取指纹编号数字在屏幕上的 X 坐标，如果没有计算过则计算后返回。
 * @param screen 屏幕
 * @param fingerIdXPtr 坐标的指针
 * @return X 坐标
 */
static uint8_t getOrComputeFingerIdX(Screen &screen, uint8_t *const fingerIdXPtr) {
    uint8_t fingerIdX;
    if (fingerIdXPtr == nullptr || *fingerIdXPtr == 0) {
        fingerIdX = screen.computeStringWidth(F("录入指纹#")) + 2;
        *fingerIdXPtr = fingerIdX;
    } else {
        fingerIdX = *fingerIdXPtr;
    }
    return fingerIdX;
}

static void displayEnrollScreen(uint8_t fingerId, Screen *screen, uint8_t *const fingerIdXPtr,
                                const __FlashStringHelper *title, const __FlashStringHelper *summary = nullptr) {
    if (screen == nullptr) return;

    screen->setPowerSave(false);

    screen->u8g2.firstPage();
    do {
        screen->setFontSize(12);
        screen->draw(F("录入指纹#"), 0, 0);
        screen->u8g2.setCursor(getOrComputeFingerIdX(*screen, fingerIdXPtr), 12);
        screen->u8g2.print(fingerId);
        screen->setFontSize(14);
        screen->drawCenter(title);
        if (summary != nullptr) {
            screen->setFontSize(12);
            screen->drawEndHorizontal(summary, Screen::HEIGHT - 12);
        }
    } while (screen->u8g2.nextPage());
}

int8_t Finger::enroll(uint8_t id, Screen *screen) {
    uint8_t fingerIdX = 0;
    int status = -1;

    // 第一轮指纹收集
    while (status != FINGERPRINT_OK) {
        status = finger.getImage();
        switch (status) {
            case FINGERPRINT_OK:
                break;
            case FINGERPRINT_NOFINGER:
                displayEnrollScreen(id, screen, &fingerIdX, F("按下手指"), F("正在等待…"));
                break;
            case FINGERPRINT_PACKETRECIEVEERR:
                displayEnrollScreen(id, screen, &fingerIdX, F("通信错误"), F("请重试"));
                break;
            case FINGERPRINT_IMAGEFAIL:
                displayEnrollScreen(id, screen, &fingerIdX, F("成像错误"), F("请重试"));
                break;
            default:
                displayEnrollScreen(id, screen, &fingerIdX, F("未知错误"), F("请重试"));
                break;
        }
    }

    // 转换指纹信息
    displayEnrollScreen(id, screen, &fingerIdX, F("成像中"), F("请等待…"));
    status = finger.image2Tz(1);
    switch (status) {
        case FINGERPRINT_OK:
            break;
        case FINGERPRINT_IMAGEMESS:
            displayEnrollScreen(id, screen, &fingerIdX, F("图像过于混乱"), F("建议清洁传感器"));
            return IMAGE_MESS;
        case FINGERPRINT_PACKETRECIEVEERR:
            displayEnrollScreen(id, screen, &fingerIdX, F("通信错误"));
            return PACKET_RECEIVE_ERROR;
        case FINGERPRINT_FEATUREFAIL:
            displayEnrollScreen(id, screen, &fingerIdX, F("无法确定指纹特征"));
            return FEATURE_FAIL;
        case FINGERPRINT_INVALIDIMAGE:
            displayEnrollScreen(id, screen, &fingerIdX, F("无法确定指纹特征"));
            return INVALID_IMAGE;
        default:
            displayEnrollScreen(id, screen, &fingerIdX, F("未知错误"));
            return UNKNOWN_ERROR;
    }

    // 等待移开手指
    displayEnrollScreen(id, screen, &fingerIdX, F("移开手指"), F("正在等待…"));
    finger.LEDcontrol(false);
    while (isFingerPressed()) delay(50);
    status = 0;
    while (status != FINGERPRINT_NOFINGER) status = finger.getImage();
    finger.LEDcontrol(true);

    // 第二轮收集
    status = -1;
    while (status != FINGERPRINT_OK) {
        status = finger.getImage();
        switch (status) {
            case FINGERPRINT_OK:
                break;
            case FINGERPRINT_NOFINGER:
                displayEnrollScreen(id, screen, &fingerIdX, F("按下同个手指"), F("正在等待…"));
                break;
            case FINGERPRINT_PACKETRECIEVEERR:
                displayEnrollScreen(id, screen, &fingerIdX, F("通信错误"), F("请重试"));
                break;
            case FINGERPRINT_IMAGEFAIL:
                displayEnrollScreen(id, screen, &fingerIdX, F("成像错误"), F("请重试"));
                break;
            default:
                displayEnrollScreen(id, screen, &fingerIdX, F("未知错误"), F("请重试"));
                break;
        }
    }

    // 转换
    displayEnrollScreen(id, screen, &fingerIdX, F("成像中"), F("请等待…"));
    status = finger.image2Tz(2);
    switch (status) {
        case FINGERPRINT_OK:
            break;
        case FINGERPRINT_IMAGEMESS:
            displayEnrollScreen(id, screen, &fingerIdX, F("图像过于混乱"), F("建议清洁传感器"));
            return IMAGE_MESS;
        case FINGERPRINT_PACKETRECIEVEERR:
            displayEnrollScreen(id, screen, &fingerIdX, F("通信错误"));
            return PACKET_RECEIVE_ERROR;
        case FINGERPRINT_FEATUREFAIL:
            displayEnrollScreen(id, screen, &fingerIdX, F("无法确定指纹特征"));
            return FEATURE_FAIL;
        case FINGERPRINT_INVALIDIMAGE:
            displayEnrollScreen(id, screen, &fingerIdX, F("无法确定指纹特征"));
            return INVALID_IMAGE;
        default:
            displayEnrollScreen(id, screen, &fingerIdX, F("未知错误"));
            return UNKNOWN_ERROR;
    }

    // 建模
    displayEnrollScreen(id, screen, &fingerIdX, F("建模中"), F("请等待…"));
    status = finger.createModel();
    switch (status) {
        case FINGERPRINT_OK:
            break;
        case FINGERPRINT_PACKETRECIEVEERR:
            displayEnrollScreen(id, screen, &fingerIdX, F("通信错误"));
            return PACKET_RECEIVE_ERROR;
        case FINGERPRINT_ENROLLMISMATCH:
            displayEnrollScreen(id, screen, &fingerIdX, F("两次指纹不匹配"));
            return ENROLL_MISMATCH;
        default:
            displayEnrollScreen(id, screen, &fingerIdX, F("未知错误"));
            return UNKNOWN_ERROR;
    }

    // 存储
    status = finger.storeModel(id);
    switch (status) {
        case FINGERPRINT_OK:
            break;
        case FINGERPRINT_PACKETRECIEVEERR:
            displayEnrollScreen(id, screen, &fingerIdX, F("通信错误"));
            return PACKET_RECEIVE_ERROR;
        case FINGERPRINT_BADLOCATION:
            displayEnrollScreen(id, screen, &fingerIdX, F("存储失败"), F("存储地址不可用"));
            return BAD_LOCATION;
        case FINGERPRINT_FLASHERR:
            displayEnrollScreen(id, screen, &fingerIdX, F("存储失败"), F("无法写入Flash"));
            return FLASH_ERROR;
        default:
            displayEnrollScreen(id, screen, &fingerIdX, F("未知错误"));
            return UNKNOWN_ERROR;
    }

    displayEnrollScreen(id, screen, &fingerIdX, F("成功"), F("现在可移开手指"));
    return OK;
}

static void displayVerifyScreen(Screen *screen,
                                const __FlashStringHelper *title, const __FlashStringHelper *summary = nullptr) {
    if (screen == nullptr) return;

    screen->setPowerSave(false);

    uint8_t titleY = summary == nullptr ? (Screen::HEIGHT - 16) / 2 : (Screen::HEIGHT - 16 - 12 - 4) / 2;
    screen->u8g2.firstPage();
    do {
        screen->setFontSize(16);
        screen->drawCenterHorizontal(title, titleY);
        if (summary != nullptr) {
            screen->setFontSize(12);
            screen->drawCenterHorizontal(summary, 16 + 4 + (Screen::HEIGHT - 16 - 12 - 4) / 2);
        }
    } while (screen->u8g2.nextPage());
}

int32_t Finger::verify(Screen *screen) {
    uint8_t status = finger.getImage();
    switch (status) {
        case FINGERPRINT_OK:
            break;
        case FINGERPRINT_NOFINGER:
            return NO_FINGER;
        case FINGERPRINT_PACKETRECIEVEERR:
            displayVerifyScreen(screen, F("通信错误"));
            return PACKET_RECEIVE_ERROR;
        case FINGERPRINT_IMAGEFAIL:
            displayVerifyScreen(screen, F("成像错误"));
            return IMAGE_FAIL;
        default:
            displayVerifyScreen(screen, F("未知错误"));
            return UNKNOWN_ERROR;
    }

    displayVerifyScreen(screen, F("识别中"), F("请等待…"));
    status = finger.image2Tz();
    switch (status) {
        case FINGERPRINT_OK:
            break;
        case FINGERPRINT_IMAGEMESS:
            displayVerifyScreen(screen, F("图像过于混乱"), F("建议清洁传感器"));
            return IMAGE_MESS;
        case FINGERPRINT_PACKETRECIEVEERR:
            displayVerifyScreen(screen, F("通信错误"));
            return PACKET_RECEIVE_ERROR;
        case FINGERPRINT_FEATUREFAIL:
            displayVerifyScreen(screen, F("无法确定指纹特征"));
            return FEATURE_FAIL;
        case FINGERPRINT_INVALIDIMAGE:
            displayVerifyScreen(screen, F("无法确定指纹特征"));
            return INVALID_IMAGE;
        default:
            displayVerifyScreen(screen, F("未知错误"));
            return UNKNOWN_ERROR;
    }

    status = finger.fingerSearch();
    switch (status) {
        case FINGERPRINT_OK:
            break;
        case FINGERPRINT_PACKETRECIEVEERR:
            displayVerifyScreen(screen, F("通信错误"));
            return PACKET_RECEIVE_ERROR;
        case FINGERPRINT_NOTFOUND:
            displayVerifyScreen(screen, F("失败"), F("无匹配指纹"));
            return NOT_FOUND;
        default:
            displayVerifyScreen(screen, F("未知错误"));
            return UNKNOWN_ERROR;
    }

    // Serial.print(F("Found ID #"));
    // Serial.println(finger.fingerID);
    // Serial.print(F(" with confidence of "));
    // Serial.println(finger.confidence);

    displayVerifyScreen(screen, F("完成"), F("欢迎回家"));
    return finger.fingerID;
}

static void displayRemoveScreen(uint8_t fingerId, Screen *screen, uint8_t *const fingerIdXPtr,
                                const __FlashStringHelper *title, const __FlashStringHelper *summary = nullptr) {
    if (screen == nullptr) return;

    screen->setPowerSave(false);

    screen->u8g2.firstPage();
    do {
        screen->setFontSize(12);
        screen->draw(F("删除指纹#"), 0, 12);
        screen->u8g2.setCursor(getOrComputeFingerIdX(*screen, fingerIdXPtr), 12);
        screen->u8g2.print(fingerId);
        screen->setFontSize(14);
        screen->drawCenterHorizontal(title, (Screen::HEIGHT - 12 - 14) / 2);
        if (summary != nullptr) {
            screen->setFontSize(12);
            screen->drawEndHorizontal(summary, Screen::HEIGHT - 12);
        }
    } while (screen->u8g2.nextPage());
}

uint8_t Finger::remove(uint8_t id, Screen *screen) {
    uint8_t fingerIdX = 0;
    uint8_t status = finger.deleteModel(id);

    switch (status) {
        case FINGERPRINT_OK:
            break;
        case FINGERPRINT_PACKETRECIEVEERR:
            displayRemoveScreen(id, screen, &fingerIdX, F("通信错误"));
            break;
        case FINGERPRINT_BADLOCATION:
            displayRemoveScreen(id, screen, &fingerIdX, F("删除失败"), F("存储地址不可用"));
            break;
        case FINGERPRINT_FLASHERR:
            displayRemoveScreen(id, screen, &fingerIdX, F("删除失败"), F("无法写入Flash"));
            break;
        default:
            // TODO: 显示错误码
            displayRemoveScreen(id, screen, &fingerIdX, F("未知错误"));
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
