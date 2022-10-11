#ifndef WATCHDOOG_ACTIVE_BUZZER_H
#define WATCHDOOG_ACTIVE_BUZZER_H

#include <Arduino.h>

class ActiveBuzzer {
public:
    ActiveBuzzer(uint8_t pin);

    void setup() const;

    void success() const;

    void warning() const;

    void boot() const;

private:
    int16_t pin = -1;
};


#endif //WATCHDOOG_ACTIVE_BUZZER_H
