#ifndef WATCHDOOG_PASSWORDS_H
#define WATCHDOOG_PASSWORDS_H

#include <stddef.h>
#include "io/data.h"
#include "io/keyboard_4_x_4.h"
#include "display/ssd_1306_oled.h"

class Passwords {
public:
    static void setup();

    static uint8_t getCount();

    static int add(Keyboard &keyboard, Screen *screen);

    static int remove(Keyboard &keyboard, Screen *screen);

    static uint8_t verify(const char *password);

    enum Status {
        SUCCESS,
        NOT_FOUND,
    };
};

#endif //WATCHDOOG_PASSWORDS_H
