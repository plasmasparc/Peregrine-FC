#include "IMU_Manager.h"
#include "GPS_Manager.h"
#include "GPSNav.h"

GPSNav gpsNav;

uint32_t last_update_time = 0;
uint32_t last_print_time = 0;

void setup() {
    Serial.begin(115200);
    delay(2000);
    
    Serial.println("GPS-NAV Test");
    
    // Direct initialization
    initMPU6050();
    startIMU();
    Serial.println("IMU OK");
    
    startGPS();
    Serial.println("GPS OK");
    
    delay(1000);
    
    // Wait for GPS fix
    Serial.println("Waiting for GPS fix...");
    while(!hasGPSFix()) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nGPS fix acquired");
    
    // Get current position and set waypoint
    double lat, lon;
    float alt, speed, course;
    uint8_t hour, minute, seconds, satellites;
    bool fix;
    
    getAllGPSDataV1(&lat, &lon, &alt, &speed, &hour, &minute, &seconds, 
                    &satellites, &fix, &course);
    
    // Waypoint 0.001 deg north (~111m)
    gpsNav.setWaypoint(lat + 0.001, lon);
    gpsNav.enableNavigation(true);
    
    Serial.print("Position: ");
    Serial.print(lat, 7);
    Serial.print(", ");
    Serial.println(lon, 7);
    
    Serial.print("Waypoint: ");
    Serial.print(lat + 0.001, 7);
    Serial.print(", ");
    Serial.println(lon, 7);
    
    last_update_time = millis();
    last_print_time = millis();
}

void loop() {
    uint32_t now = millis();
    
    // Update at 20Hz
    if(now - last_update_time >= 50) {
        gpsNav.update();
        last_update_time = now;
    }
    
    // Print at 2Hz
    if(now - last_print_time >= 500) {
        printData();
        last_print_time = now;
    }
}

void printData() {
    double lat, lon;
    float alt, speed, course;
    uint8_t hour, minute, seconds, satellites;
    bool fix;
    
    getAllGPSDataV1(&lat, &lon, &alt, &speed, &hour, &minute, &seconds, 
                    &satellites, &fix, &course);
    
    float roll, pitch, yaw;
    float rate_x, rate_y, rate_z;
    getAllIMUData(&roll, &pitch, &yaw, &rate_x, &rate_y, &rate_z);
    
    Serial.println("================================");
    Serial.print("GPS Fix: ");
    Serial.print(fix ? "YES" : "NO");
    Serial.print(" | Sats: ");
    Serial.println(satellites);
    
    Serial.print("Speed: ");
    Serial.print(speed, 2);
    Serial.print(" m/s | GPS Course: ");
    Serial.print(course, 1);
    Serial.println(" deg");
    
    Serial.print("IMU Rate Z: ");
    Serial.print(rate_z * 57.29578f, 2);
    Serial.println(" deg/s");
    
    Serial.print("Fused Heading: ");
    Serial.print(gpsNav.getHeading(), 2);
    Serial.print(" deg | Rate: ");
    Serial.print(gpsNav.getHeadingRate(), 2);
    Serial.println(" deg/s");
    
    Serial.print("Heading Bias: ");
    Serial.print(gpsNav.getHeadingRate() - rate_z * 57.29578f, 3);
    Serial.println(" deg/s");
    
    Serial.print("Yaw Command: ");
    Serial.print(gpsNav.getYawRateCommand(), 2);
    Serial.println(" deg/s");
    
    Serial.print("Valid: ");
    Serial.println(gpsNav.isValid() ? "YES" : "NO");
}