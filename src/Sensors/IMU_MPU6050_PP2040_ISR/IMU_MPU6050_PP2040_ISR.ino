#include "IMU_Manager.h"

void setup() {
    Serial.begin(115200);
    
    // Start IMU -> automatic updates at 666Hz
    startIMU();
}

void loop() {
    float roll, pitch, yaw;
    float rate_x, rate_y, rate_z;
    
    // Option 1: Get attitude
    getAttitude(&roll, &pitch, &yaw);
    
    // Option 2: Get everything in one
    getAllIMUData(&roll, &pitch, &yaw, &rate_x, &rate_y, &rate_z);
    
    // Use data
    //Serial.print("Roll: "); Serial.print(roll);
    //Serial.print(" Pitch: "); Serial.print(pitch);
    //Serial.print(" Yaw: "); Serial.println(yaw);

    Serial.print("rate_x: "); Serial.print(rate_x*1000);
    Serial.print(" rate_y: "); Serial.print(rate_y*1000);
    Serial.print(" rate_z: "); Serial.println(rate_z*1000);
    
    delay(10);
}
