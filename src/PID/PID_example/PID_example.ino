#include "IMU_Manager.h"
#include "PID_Controller.h"

void setup() {
    Serial.begin(115200);
    
    // Initialize IMU
    startIMU();
    delay(1000);
    
    // Initialize PID controller
    startPID();
    
    // Set custom gains if needed
    // setPitchGains(0.5f, 0.0f, 0.15f);
    // setRollGains(0.5f, 0.0f, 0.15f);
    // setYawGains(0.5f, 0.0f, 0.15f);
    
    // Set initial targets
    setTargetRoll(0.0f);
    setTargetPitch(0.0f);
    setTargetYaw(0.0f);
}

void loop() {
    float roll, pitch, yaw;
    getAttitude(&roll, &pitch, &yaw);
    
    Serial.print(roll);
    Serial.print(",");
    Serial.print(pitch);
    Serial.print(",");
    Serial.println(yaw);
    
    delay(100);
}
