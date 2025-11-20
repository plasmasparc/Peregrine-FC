#include "IMU_Manager.h"
#include "GPS_Manager.h"

void setup() {
    Serial.begin(115200);
    
    // Start both managers with independent ISRs
    startIMU();  // 1.5ms updates on ALARM 0
    startGPS();  // 500ms updates on ALARM 1
}

void loop() {
    // Get IMU data
    float roll, pitch, yaw;
    float rate_x, rate_y, rate_z;
    getAllIMUData(&roll, &pitch, &yaw, &rate_x, &rate_y, &rate_z);
    
    // Get GPS data
    double lat, lon;
    float alt, speed;
    uint8_t hour, minute, seconds, satellites;
    bool fix;
    getAllGPSData(&lat, &lon, &alt, &speed, &hour, &minute, &seconds, &satellites, &fix);
    
    // Print IMU data
    Serial.print("IMU - Roll: "); Serial.print(roll);
    Serial.print(" Pitch: "); Serial.print(pitch);
    Serial.print(" Yaw: "); Serial.println(yaw);
    
    // Print GPS data
    if (fix) {
        Serial.print("GPS - Lat: "); Serial.print(lat, 6);
        Serial.print(" Lon: "); Serial.print(lon, 6);
        Serial.print(" Alt: "); Serial.print(alt);
        Serial.print(" Sats: "); Serial.println(satellites);
    } else {
        Serial.println("GPS - No fix");
    }
    
    Serial.println();
    delay(1000);
}