#include "GPS_Manager.h"

void setup() {
    Serial.begin(115200);
    startGPS();  // 500ms updates on ALARM 1
}

void loop() {
    // Get GPS data
    double lat, lon;
    float alt, speed;
    uint8_t hour, minute, seconds, satellites;
    bool fix;
    getAllGPSData(&lat, &lon, &alt, &speed, &hour, &minute, &seconds, &satellites, &fix);
    
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
