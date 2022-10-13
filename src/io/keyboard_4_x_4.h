#ifndef WATCHDOOG_KEYBOARD_4_X_4_H
#define WATCHDOOG_KEYBOARD_4_X_4_H

#include <Arduino.h>
#include <Keypad.h>

class Keyboard {
public:
    static const uint8_t ROWS = 4;
    static const uint8_t COLS = 4;
    // constexpr static const char KEYS[ROWS][COLS] = {
    //         {'1', '2', '3', 'A'},
    //         {'4', '5', '6', 'B'},
    //         {'7', '8', '9', 'C'},
    //         {'*', '0', '#', 'D'}
    // };

    Keyboard(uint8_t colPins[4], uint8_t rowPins[4]);

    void setup();

    void tick();

    void clear();

    char getPressedKey();

    boolean hasNewInput() const;

    void reserve(size_t capacity);

    void shrink();

    const char *getInput();

private:
    Keypad keypad;

    char *in = nullptr;
    size_t inLength = 0;
    size_t inCapacity = 0;
};

#endif //WATCHDOOG_KEYBOARD_4_X_4_H
