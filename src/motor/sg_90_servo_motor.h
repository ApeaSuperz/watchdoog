#ifndef WATCHDOOG_SG_90_SERVO_MOTOR_H
#define WATCHDOOG_SG_90_SERVO_MOTOR_H

#include <Arduino.h>

class ServoMotor {
public:
    explicit ServoMotor(uint8_t pin);

    void setup();

    void rotate(int angle);

private:
    uint8_t pin;

    void servoWithoutShake(int angle) const;
};

#endif //WATCHDOOG_SG_90_SERVO_MOTOR_H
