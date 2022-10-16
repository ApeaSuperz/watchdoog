#include "passwords.h"

static uint8_t count;

void Passwords::setup() {
    Data::setup();
    count = Data::getInt("passwords.count");
}

uint8_t Passwords::getCount() {
    return count;
}

int Passwords::add(Keyboard &keyboard, Screen *screen) {
    if (screen != nullptr) {
        screen->setPowerSave(false);
        screen->u8g2.firstPage();
        do {
            screen->setFontSize(12);
            screen->drawCenterHorizontal(("添加密码"), 0);
            screen->drawCenterHorizontal(("按任意非数字键结束"), Screen::HEIGHT - 12);
            screen->setFontSize(16);
            screen->drawCenter(("请输入密码"));
        } while (screen->u8g2.nextPage());
    }

    while (true) {
        if (keyboard.hasNewInput()) {
            if (keyboard.getInput()[keyboard.getInputLength()] < '0' ||
                keyboard.getInput()[keyboard.getInputLength()] > '9') {
                break;
            }
            if (screen != nullptr) {
                screen->u8g2.firstPage();
                do {
                    screen->setFontSize(12);
                    screen->drawCenterHorizontal(("添加密码"), 0);
                    screen->drawCenterHorizontal(("按任意非数字键结束"), Screen::HEIGHT - 12);
                    screen->setFontSize(16);
                    screen->drawCenter(keyboard.getInput());
                } while (screen->u8g2.nextPage());
            }
        }
    }

    count++;
    Data::save(("passwords." + String(count)).c_str(), keyboard.getInput());
    Data::save("passwords.count", count);

    return SUCCESS;
}

int Passwords::remove(Keyboard &keyboard, Screen *screen) {
    if (screen != nullptr) {
        screen->setPowerSave(false);
        screen->u8g2.firstPage();
        do {
            screen->setFontSize(12);
            screen->drawCenterHorizontal(("删除密码"), 0);
            screen->drawCenterHorizontal(("按任意非数字键结束"), Screen::HEIGHT - 12);
            screen->setFontSize(16);
            screen->drawCenter(("请输入密码"));
        } while (screen->u8g2.nextPage());
    }

    while (true) {
        if (keyboard.hasNewInput()) {
            if (keyboard.getInput()[keyboard.getInputLength()] < '0' ||
                keyboard.getInput()[keyboard.getInputLength()] > '9') {
                break;
            }
            if (screen != nullptr) {
                screen->u8g2.firstPage();
                do {
                    screen->setFontSize(12);
                    screen->drawCenterHorizontal(("删除密码"), 0);
                    screen->drawCenterHorizontal(("按任意非数字键结束"), Screen::HEIGHT - 12);
                    screen->setFontSize(16);
                    screen->drawCenter(keyboard.getInput());
                } while (screen->u8g2.nextPage());
            }
        }
    }

    for (uint8_t i = 1; i <= count; i++) {
        if (strcmp(keyboard.getInput(), Data::getString(("passwords." + String(i)).c_str())) == 0) {
            if (i != count) {
                for (uint8_t j = i; j < count; j++) {
                    Data::save(("passwords." + String(j)).c_str(),
                               Data::getString(("passwords." + String(j + 1)).c_str()));
                }
            }
            Data::del(("passwords." + String(count)).c_str());
            count--;
            Data::save("passwords.count", count);
            return SUCCESS;
        }
    }

    // 没找到输入的密码
    return NOT_FOUND;
}

uint8_t Passwords::verify(const char *password) {
    for (uint8_t i = 1; i <= count; i++) {
        if (strcmp(password, Data::getString(("passwords." + String(i)).c_str())) == 0) {
            return i;
        }
    }
    return 0;
}
