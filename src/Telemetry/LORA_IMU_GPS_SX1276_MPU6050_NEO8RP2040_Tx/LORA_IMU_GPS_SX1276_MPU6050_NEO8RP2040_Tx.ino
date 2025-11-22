#include "LoRaRadio.h"
#include "IMU_Manager.h"
#include "GPS_Manager.h"

LoRaRadio lora;

void setup() {
    Serial.begin(115200);
    
    // Initialize sensors
    startIMU();
    startGPS();
    
    // Initialize LoRa with default config
    if (lora.init()) {
        Serial.println("LoRa initialized");
    } else {
        Serial.println("LoRa init failed!");
        while(1);
    }
}

void loop() {
    // Get sensor data
    float roll, pitch, yaw;
    getAttitude(&roll, &pitch, &yaw);
    
    double lat, lon;
    float alt, speed;
    uint8_t hour, minute, seconds, satellites;
    bool fix;
    getAllGPSData(&lat, &lon, &alt, &speed, &hour, &minute, &seconds, &satellites, &fix);
    
    // Build telemetry message
    String telemetry = "";
    
    // IMU data
    telemetry += "R:";
    telemetry += String(roll, 1);
    telemetry += ",P:";
    telemetry += String(pitch, 1);
    telemetry += ",Y:";
    telemetry += String(yaw, 1);
    
    // GPS data
    if (fix) {
        telemetry += ",LAT:";
        telemetry += String(lat, 6);
        telemetry += ",LON:";
        telemetry += String(lon, 6);
        telemetry += ",ALT:";
        telemetry += String(alt, 1);
        telemetry += ",SPD:";
        telemetry += String(speed, 1);
        telemetry += ",SAT:";
        telemetry += String(satellites);
    } else {
        telemetry += ",GPS:NOFIX";
    }
    
    // Calculate and print airtime
    float airtime = lora.calculateAirtime(telemetry.length());
    
    Serial.println("Sending telemetry:");
    Serial.println(telemetry);
    Serial.print("Message length: ");
    Serial.print(telemetry.length());
    Serial.println(" bytes");
    Serial.print("Airtime: ");
    Serial.print(airtime, 1);
    Serial.println(" ms");
    
    // Send telemetry
    lora.send(telemetry);
    
    Serial.println();
    delay(1000);
}
