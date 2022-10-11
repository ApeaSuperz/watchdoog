#include "active_buzzer.h"

ActiveBuzzer::ActiveBuzzer(uint8_t pin) : pin(pin) {}

void ActiveBuzzer::setup() const {
    pinMode(pin, OUTPUT);
}

void ActiveBuzzer::success() const {
    tone(pin, 1000);
    delay(125);
    tone(pin, 1500);
    delay(125);
    noTone(pin);
}

void ActiveBuzzer::warning() const {
    tone(pin, 1000);
    delay(150);
    noTone(pin);
    delay(150);
    tone(pin, 1000);
    delay(150);
    noTone(pin);
    delay(150);
    tone(pin, 1000);
    delay(150);
    noTone(pin);
}

void ActiveBuzzer::boot() const {
    tone(pin, 262);
    delay(100);
    tone(pin, 294);
    delay(100);
    tone(pin, 330);
    delay(100);
    tone(pin, 349);
    delay(100);
    tone(pin, 392);
    delay(100);
    tone(pin, 440);
    delay(100);
    tone(pin, 494);
    delay(100);
    tone(pin, 523);
    delay(100);
    noTone(pin);
}
