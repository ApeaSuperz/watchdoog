#include "sg_90_servo_motor.h"

ServoMotor::ServoMotor(uint8_t pin) : pin(pin) {}

void ServoMotor::setup() {
    pinMode(pin, OUTPUT);
    rotate(0);
}

void ServoMotor::rotate(int angle) {
    for (int i = 0; i < 50; i++) {
        servoWithoutShake(angle);
    }
}

void ServoMotor::servoWithoutShake(int angle) const {
    int pulseWidth = (angle * 11) + 500;  // 将角度转化为 500-2480 的脉宽值，每多转 1 度，对应高电平多 11us
    digitalWrite(pin, HIGH);
    delayMicroseconds(pulseWidth);
    digitalWrite(pin, LOW);
    delayMicroseconds(20000 - pulseWidth);
}
