#include "NEO6_8.h"

NEO6_8 gps;

void setup() {
    Serial.begin(115200);
    gps.init();
}

void loop() {
    gps.update();
    
    if (gps.hasFix()) {
        // Option 1: Get full data structure
        const GPSData& data = gps.getData();
        
        Serial.print("Time: ");
        Serial.print(data.hour);
        Serial.print(":");
        Serial.print(data.minute);
        Serial.print(":");
        Serial.println(data.seconds);
        
        Serial.print("Position: ");
        Serial.print(data.latitude, 6);
        Serial.print(", ");
        Serial.println(data.longitude, 6);
        
        Serial.print("Altitude: ");
        Serial.print(data.altitude);
        Serial.println(" m");
        
        Serial.print("Speed: ");
        Serial.print(data.speed);
        Serial.println(" m/s");
        
        Serial.print("Satellites: ");
        Serial.println(data.satellites);
        
        // Option 2: Get individual values
        double lat = gps.getLatitude();
        double lon = gps.getLongitude();
        
        Serial.println();
    } else {
        Serial.println("No GPS fix");
    }
    
    delay(500);
}
