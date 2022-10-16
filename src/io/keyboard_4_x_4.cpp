#include "keyboard_4_x_4.h"

static const char KEYS[4][4] = {
        {'1', '2', '3', 'A'},
        {'4', '5', '6', 'B'},
        {'7', '8', '9', 'C'},
        {'*', '0', '#', 'D'}
};

Keyboard::Keyboard(uint8_t *colPins, uint8_t *rowPins) :
        keypad(makeKeymap(KEYS), rowPins, colPins, ROWS, COLS) {
    in = nullptr;
    inLength = 0;
    inCapacity = 0;
}

void Keyboard::setup() {
    reserve(1);
    in[0] = '\0';
}

void Keyboard::tick() {
    char key = keypad.getKey();

    if (key == NO_KEY) return;

    if (inLength == inCapacity)
        reserve(inCapacity == 0 ? 1 : inCapacity * 2);

    in[inLength - 1] = key;
    in[inLength] = '\0';
    inLength++;
}

void Keyboard::clear() {
    if (inCapacity != 0) {
        in[0] = '\0';
        inLength = 1;
        shrink();
    }
}

char Keyboard::getPressedKey() {
    return keypad.getKey();
}

boolean Keyboard::hasNewInput() const {
    static size_t lastInLength = 1;
    if (lastInLength != inLength) {
        lastInLength = inLength;
        return inLength != 1;
    }
    return false;
}

void Keyboard::reserve(size_t capacity) {
    if (inCapacity < capacity) {
        inCapacity = capacity < 2 ? 2 : capacity;
        in = static_cast<char *>(realloc(in, capacity * sizeof(char)));
    }
}

void Keyboard::shrink() {
    if (inCapacity > inLength) {
        inCapacity = inLength < 2 ? 2 : inLength;
        in = static_cast<char *>(realloc(in, inCapacity * sizeof(char)));
    }
}

const char *Keyboard::getInput() {
    return in;
}

uint8_t Keyboard::getInputLength() const {
    return inLength - 1;
}
