#include <Arduino.h>
#include "DSHOT300_ESC.h"

void setup() {
    Serial.begin(115200);
    
    escInit();
    Serial.println("ESC initialized");
    
    delay(1000);
    
    escArm();
    Serial.println("ESC armed");
    
    delay(1000);
}

void loop() {
    // Ramp test
    Serial.println("Starting ramp test");
    
    for (int i = 0; i <= 100; i++) {
        float thrust = i / 100.0f;
        escSetThrust(thrust);
        
        Serial.print("Thrust: ");
        Serial.println(thrust, 2);
        
        delay(50);
    }
    
    delay(1000);
    
    // Stop
    escSetThrust(0.0f);
    Serial.println("Motors stopped");
    
    delay(3000);
}
