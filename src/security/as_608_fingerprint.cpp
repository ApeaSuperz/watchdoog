#include "as_608_fingerprint.h"

Finger::Finger(SoftwareSerial *serial, int8_t touchPin, uint32_t password) :
        touchPin(touchPin),
        finger(Adafruit_Fingerprint(serial, password)),
        isPowerSaving(false) {}

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
        fingerIdX = screen.computeStringWidth(("录入指纹#")) + 2;
        *fingerIdXPtr = fingerIdX;
    } else {
        fingerIdX = *fingerIdXPtr;
    }
    return fingerIdX;
}

static void displayEnrollScreen(uint8_t fingerId, Screen *screen, uint8_t *const fingerIdXPtr,
                                const char *title, const char *summary = nullptr) {
    if (screen == nullptr) return;

    screen->setPowerSave(false);

    screen->u8g2.firstPage();
    do {
        screen->setFontSize(12);
        screen->draw(("录入指纹#"), 0, 0);
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
                displayEnrollScreen(id, screen, &fingerIdX, ("按下手指"), ("正在等待…"));
                break;
            case FINGERPRINT_PACKETRECIEVEERR:
                displayEnrollScreen(id, screen, &fingerIdX, ("通信错误"), ("请重试"));
                break;
            case FINGERPRINT_IMAGEFAIL:
                displayEnrollScreen(id, screen, &fingerIdX, ("成像错误"), ("请重试"));
                break;
            default:
                displayEnrollScreen(id, screen, &fingerIdX, ("未知错误"), ("请重试"));
                break;
        }
    }

    // 转换指纹信息
    displayEnrollScreen(id, screen, &fingerIdX, ("成像中"), ("请等待…"));
    status = finger.image2Tz(1);
    switch (status) {
        case FINGERPRINT_OK:
            break;
        case FINGERPRINT_IMAGEMESS:
            displayEnrollScreen(id, screen, &fingerIdX, ("图像过于混乱"), ("建议清洁传感器"));
            return IMAGE_MESS;
        case FINGERPRINT_PACKETRECIEVEERR:
            displayEnrollScreen(id, screen, &fingerIdX, ("通信错误"));
            return PACKET_RECEIVE_ERROR;
        case FINGERPRINT_FEATUREFAIL:
            displayEnrollScreen(id, screen, &fingerIdX, ("无法确定指纹特征"));
            return FEATURE_FAIL;
        case FINGERPRINT_INVALIDIMAGE:
            displayEnrollScreen(id, screen, &fingerIdX, ("无法确定指纹特征"));
            return INVALID_IMAGE;
        default:
            displayEnrollScreen(id, screen, &fingerIdX, ("未知错误"));
            return UNKNOWN_ERROR;
    }

    // 等待移开手指
    displayEnrollScreen(id, screen, &fingerIdX, ("移开手指"), ("正在等待…"));
    setPowerSave(true);
    while (isFingerPressed()) delay(50);
    status = 0;
    while (status != FINGERPRINT_NOFINGER) status = finger.getImage();
    setPowerSave(false);

    // 第二轮收集
    status = -1;
    while (status != FINGERPRINT_OK) {
        status = finger.getImage();
        switch (status) {
            case FINGERPRINT_OK:
                break;
            case FINGERPRINT_NOFINGER:
                displayEnrollScreen(id, screen, &fingerIdX, ("按下同个手指"), ("正在等待…"));
                break;
            case FINGERPRINT_PACKETRECIEVEERR:
                displayEnrollScreen(id, screen, &fingerIdX, ("通信错误"), ("请重试"));
                break;
            case FINGERPRINT_IMAGEFAIL:
                displayEnrollScreen(id, screen, &fingerIdX, ("成像错误"), ("请重试"));
                break;
            default:
                displayEnrollScreen(id, screen, &fingerIdX, ("未知错误"), ("请重试"));
                break;
        }
    }

    // 转换
    displayEnrollScreen(id, screen, &fingerIdX, ("成像中"), ("请等待…"));
    status = finger.image2Tz(2);
    switch (status) {
        case FINGERPRINT_OK:
            break;
        case FINGERPRINT_IMAGEMESS:
            displayEnrollScreen(id, screen, &fingerIdX, ("图像过于混乱"), ("建议清洁传感器"));
            return IMAGE_MESS;
        case FINGERPRINT_PACKETRECIEVEERR:
            displayEnrollScreen(id, screen, &fingerIdX, ("通信错误"));
            return PACKET_RECEIVE_ERROR;
        case FINGERPRINT_FEATUREFAIL:
            displayEnrollScreen(id, screen, &fingerIdX, ("无法确定指纹特征"));
            return FEATURE_FAIL;
        case FINGERPRINT_INVALIDIMAGE:
            displayEnrollScreen(id, screen, &fingerIdX, ("无法确定指纹特征"));
            return INVALID_IMAGE;
        default:
            displayEnrollScreen(id, screen, &fingerIdX, ("未知错误"));
            return UNKNOWN_ERROR;
    }

    // 建模
    displayEnrollScreen(id, screen, &fingerIdX, ("建模中"), ("请等待…"));
    status = finger.createModel();
    switch (status) {
        case FINGERPRINT_OK:
            break;
        case FINGERPRINT_PACKETRECIEVEERR:
            displayEnrollScreen(id, screen, &fingerIdX, ("通信错误"));
            return PACKET_RECEIVE_ERROR;
        case FINGERPRINT_ENROLLMISMATCH:
            displayEnrollScreen(id, screen, &fingerIdX, ("两次指纹不匹配"));
            return ENROLL_MISMATCH;
        default:
            displayEnrollScreen(id, screen, &fingerIdX, ("未知错误"));
            return UNKNOWN_ERROR;
    }

    // 存储
    status = finger.storeModel(id);
    switch (status) {
        case FINGERPRINT_OK:
            break;
        case FINGERPRINT_PACKETRECIEVEERR:
            displayEnrollScreen(id, screen, &fingerIdX, ("通信错误"));
            return PACKET_RECEIVE_ERROR;
        case FINGERPRINT_BADLOCATION:
            displayEnrollScreen(id, screen, &fingerIdX, ("存储失败"), ("存储地址不可用"));
            return BAD_LOCATION;
        case FINGERPRINT_FLASHERR:
            displayEnrollScreen(id, screen, &fingerIdX, ("存储失败"), ("无法写入Flash"));
            return FLASH_ERROR;
        default:
            displayEnrollScreen(id, screen, &fingerIdX, ("未知错误"));
            return UNKNOWN_ERROR;
    }

    displayEnrollScreen(id, screen, &fingerIdX, ("成功"), ("现在可移开手指"));
    return OK;
}

static void displayVerifyScreen(Screen *screen,
                                const char *title, const char *summary = nullptr) {
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
            displayVerifyScreen(screen, ("通信错误"));
            return PACKET_RECEIVE_ERROR;
        case FINGERPRINT_IMAGEFAIL:
            displayVerifyScreen(screen, ("成像错误"));
            return IMAGE_FAIL;
        default:
            displayVerifyScreen(screen, ("未知错误"));
            return UNKNOWN_ERROR;
    }

    displayVerifyScreen(screen, ("识别中"), ("请等待…"));
    status = finger.image2Tz();
    switch (status) {
        case FINGERPRINT_OK:
            break;
        case FINGERPRINT_IMAGEMESS:
            displayVerifyScreen(screen, ("图像过于混乱"), ("建议清洁传感器"));
            return IMAGE_MESS;
        case FINGERPRINT_PACKETRECIEVEERR:
            displayVerifyScreen(screen, ("通信错误"));
            return PACKET_RECEIVE_ERROR;
        case FINGERPRINT_FEATUREFAIL:
            displayVerifyScreen(screen, ("无法确定指纹特征"));
            return FEATURE_FAIL;
        case FINGERPRINT_INVALIDIMAGE:
            displayVerifyScreen(screen, ("无法确定指纹特征"));
            return INVALID_IMAGE;
        default:
            displayVerifyScreen(screen, ("未知错误"));
            return UNKNOWN_ERROR;
    }

    status = finger.fingerSearch();
    switch (status) {
        case FINGERPRINT_OK:
            break;
        case FINGERPRINT_PACKETRECIEVEERR:
            displayVerifyScreen(screen, ("通信错误"));
            return PACKET_RECEIVE_ERROR;
        case FINGERPRINT_NOTFOUND:
            displayVerifyScreen(screen, ("失败"), ("无匹配指纹"));
            return NOT_FOUND;
        default:
            displayVerifyScreen(screen, ("未知错误"));
            return UNKNOWN_ERROR;
    }

    // Serial.print(("Found ID #"));
    // Serial.println(security.fingerID);
    // Serial.print((" with confidence of "));
    // Serial.println(security.confidence);

    displayVerifyScreen(screen, ("完成"), ("欢迎回家"));
    return finger.fingerID;
}

static void displayRemoveScreen(uint8_t fingerId, Screen *screen, uint8_t *const fingerIdXPtr,
                                const char *title, const char *summary = nullptr) {
    if (screen == nullptr) return;

    screen->setPowerSave(false);

    screen->u8g2.firstPage();
    do {
        screen->setFontSize(12);
        screen->draw(("删除指纹#"), 0, 12);
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
            displayRemoveScreen(id, screen, &fingerIdX, ("通信错误"));
            break;
        case FINGERPRINT_BADLOCATION:
            displayRemoveScreen(id, screen, &fingerIdX, ("删除失败"), ("存储地址不可用"));
            break;
        case FINGERPRINT_FLASHERR:
            displayRemoveScreen(id, screen, &fingerIdX, ("删除失败"), ("无法写入Flash"));
            break;
        default:
            // TODO: 显示错误码
            displayRemoveScreen(id, screen, &fingerIdX, ("未知错误"));
            // Serial.print(("Unknown error: 0x"));
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

void Finger::setPowerSave(boolean state) {
    if (state != isPowerSaving) {
        finger.LEDcontrol(!state);
        isPowerSaving = state;
    }
}

boolean Finger::isPowerSave() const {
    return isPowerSaving;
}
