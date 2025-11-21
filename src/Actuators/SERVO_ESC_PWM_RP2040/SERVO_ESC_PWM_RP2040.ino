#include <Arduino.h>
#include "PWM_Controller.h"

#define CH_RUDDER 0
#define CH_ELEVATOR 1
#define CH_AILERON_L 2
#define CH_AILERON_R 3

PWMController controller(4);

void setup() {
    Serial.begin(115200);
    
    // Configure servos
    PWMConfig rudder = PWMPresets::standardServo(10);
    rudder.type = RUDDER;
    rudder.trim_us = 20;  // Trim example
    
    PWMConfig elevator = PWMPresets::standardServo(11);
    elevator.type = ELEVATOR;
    
    PWMConfig aileron_l = PWMPresets::customServo(12, 1000, 2500);
    aileron_l.type = AILERON_LEFT;
    
    PWMConfig aileron_r = PWMPresets::customServo(13, 1000, 2500);
    aileron_r.type = AILERON_RIGHT;
    aileron_r.reversed = true;  // Mirror movement
    
    controller.addChannel(CH_RUDDER, rudder);
    controller.addChannel(CH_ELEVATOR, elevator);
    controller.addChannel(CH_AILERON_L, aileron_l);
    controller.addChannel(CH_AILERON_R, aileron_r);
    
    // Center all
    controller.centerAll();
    
    Serial.println("Servos initialized and centered");
}

void loop() {
    // Test rudder
    Serial.println("Testing rudder");
    controller.getChannel(CH_RUDDER)->setNormalized(-1.0f);
    delay(1000);
    controller.getChannel(CH_RUDDER)->setNormalized(1.0f);
    delay(1000);
    controller.getChannel(CH_RUDDER)->setCenter();
    delay(1000);
    
    // Test elevator
    Serial.println("Testing elevator");
    controller.getChannel(CH_ELEVATOR)->setAngle(0);
    delay(1000);
    controller.getChannel(CH_ELEVATOR)->setAngle(180);
    delay(1000);
    controller.getChannel(CH_ELEVATOR)->setAngle(90);
    delay(1000);
    
    // Test ailerons (synchronized roll)
    Serial.println("Testing ailerons");
    controller.getChannel(CH_AILERON_L)->setNormalized(0.5f);
    controller.getChannel(CH_AILERON_R)->setNormalized(0.5f);
    delay(1000);
    controller.getChannel(CH_AILERON_L)->setNormalized(-0.5f);
    controller.getChannel(CH_AILERON_R)->setNormalized(-0.5f);
    delay(1000);
    
    controller.centerAll();
    delay(2000);
}
