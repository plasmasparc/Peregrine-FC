#include "PWM_Controller.h"

#define CH_SERVO1 0
#define CH_SERVO2 1
#define CH_SERVO3 2
#define CH_MOTOR 3

PWMController controller(4);

void setup() {
    Serial.begin(115200);
    
    PWMConfig servo1 = PWMPresets::customServo(18, 0, 2500);
    servo1.trim_us = 0;
    servo1.reversed = true;
    
    PWMConfig servo2 = PWMPresets::customServo(19, 0, 2500);
    servo2.trim_us = 0;
    //servo2.reversed = true;
    
    PWMConfig servo3 = PWMPresets::customServo(20, 0, 2500);
    servo3.trim_us = 0;
    servo3.reversed = true;
    
    PWMConfig motor = PWMPresets::customESC(21, 1000, 2000);
    
    controller.addChannel(CH_SERVO1, servo1);
    controller.addChannel(CH_SERVO2, servo2);
    controller.addChannel(CH_SERVO3, servo3);
    controller.addChannel(CH_MOTOR, motor);
    
    //controller.calibrateESC(CH_MOTOR);
    //controller.armESC(CH_MOTOR);
    
    controller.centerAll();
}

void loop() {
    controller.centerAll();
    delay(1000);
    controller.getChannel(CH_SERVO2)->setNormalized(0.7f);
    delay(1000);
    
    //while(1);
    /*
    controller.getChannel(CH_SERVO1)->setNormalized(0.5f);
    delay(1000);
    controller.getChannel(CH_SERVO1)->setNormalized(0.7f);
    delay(1000);
    controller.getChannel(CH_SERVO1)->setNormalized(0.5f);
    delay(1000);
    controller.getChannel(CH_SERVO1)->setNormalized(0.f);
    delay(1000);
    controller.getChannel(CH_SERVO1)->setNormalized(-0.5f);
    delay(1000);
    controller.getChannel(CH_SERVO1)->setNormalized(-0.7f);
    delay(1000);
    */
    //controller.getChannel(CH_SERVO2)->setNormalized(-0.5f);
    //controller.getChannel(CH_SERVO3)->setNormalized(0.3f);
    
    //controller.getChannel(CH_MOTOR)->setNormalized(0.25f);
    
    //delay(2000);
    
    //controller.centerAll();
    //delay(2000);
}
