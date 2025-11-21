#include "QMC5883_Manager.h"

void setup() {
    startMagnetometer();   // 100ms updates on ALARM 2 (10Hz)
}

void loop() {
    // Magnetometer data
    int16_t mag_x, mag_y, mag_z;
    float heading;
    getAllMagData(&mag_x, &mag_y, &mag_z, &heading);
    
    // Print Magnetometer
    Serial.print("MAG - X:");
    Serial.print(mag_x);
    Serial.print(" Y:");
    Serial.print(mag_y);
    Serial.print(" Z:");
    Serial.print(mag_z);
    Serial.print(" Heading:");
    Serial.print(heading, 1);
    Serial.println("°");
    
    Serial.println();
    delay(1000);
}
