#include "QMC5883.h"

QMC5883 compass;

void setup() {
    Serial.begin(115200);
    compass.init();
}

void loop() {
    // Update magnetometer data
    compass.update();
    
    // Option 1: Get raw data
    const MagData& data = compass.getData();
    
    Serial.print("X: ");
    Serial.print(data.x);
    Serial.print("\tY: ");
    Serial.print(data.y);
    Serial.print("\tZ: ");
    Serial.print(data.z);
    Serial.print("\tHeading: ");
    Serial.print(data.heading, 1);
    Serial.println("°");
    
    // Option 2: Get raw data
    int16_t x = compass.getX();
    int16_t y = compass.getY();
    int16_t z = compass.getZ();
    
    delay(100);
}
