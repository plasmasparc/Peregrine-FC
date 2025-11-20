#include "MPU6050.h"
void setup() {
  initMPU6050();
}

float yaw,pitch,roll;
void loop() {
  updateMPU6050();
  getRollPitchYaw(&roll, &pitch, &yaw);
  Serial.print(roll); Serial.print(" "); 
  Serial.print(pitch); Serial.print(" "); 
  Serial.println(yaw);
  delay(1);
  
}
