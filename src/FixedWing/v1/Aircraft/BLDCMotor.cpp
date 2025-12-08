// BLDCMotor.cpp
#include "BLDCMotor.h"

BLDCMotor::BLDCMotor(uint8_t pin, uint16_t min_us, uint16_t max_us) 
    : pin(pin), min_us(min_us), max_us(max_us) {}

void BLDCMotor::init() {
    esc.attach(pin, min_us, max_us);
    calibrate();
}

void BLDCMotor::calibrate() {
    delay(1000);
    esc.writeMicroseconds(max_us);
    delay(4000);
    esc.writeMicroseconds(min_us);
    delay(4000);
}

void BLDCMotor::setSpeed(uint16_t speed) {
    if (speed > 1000) speed = 1000;
    uint16_t us = min_us + speed;
    if (us > max_us) us = max_us;
    esc.writeMicroseconds(us);
}

void BLDCMotor::stop() {
    esc.writeMicroseconds(min_us);
}
