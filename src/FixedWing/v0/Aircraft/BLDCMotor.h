// BLDCMotor.h
#ifndef BLDC_MOTOR_H
#define BLDC_MOTOR_H

#include <Servo.h>

class BLDCMotor {
public:
    BLDCMotor(uint8_t pin, uint16_t min_us, uint16_t max_us);
    
    void init();
    void calibrate();
    void setSpeed(uint16_t speed);
    void stop();
    
private:
    Servo esc;
    uint8_t pin;
    uint16_t min_us;
    uint16_t max_us;
};

#endif
