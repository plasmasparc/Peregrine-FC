#include "TTGO_LoRa_Receiver.h"

T3LoRaReceiver receiver;

void setup() {
    receiver.init();
}

void loop() {
    if (receiver.receive()) {
        const TelemetryData& data = receiver.getData();
        
        receiver.displayOnOLED();
        
        Serial.println("=== Telemetry ===");
        Serial.print("Roll: "); Serial.println(data.roll);
        Serial.print("Pitch: "); Serial.println(data.pitch);
        Serial.print("Yaw: "); Serial.println(data.yaw);
        
        if (data.gps_fix) {
            Serial.print("Lat: "); Serial.println(data.latitude, 6);
            Serial.print("Lon: "); Serial.println(data.longitude, 6);
            Serial.print("Alt: "); Serial.println(data.altitude);
            Serial.print("Speed: "); Serial.println(data.speed);
            Serial.print("Sats: "); Serial.println(data.satellites);
        } else {
            Serial.println("GPS: No fix");
        }
        
        Serial.print("RSSI: "); Serial.println(data.rssi);
        Serial.print("SNR: "); Serial.println(data.snr);
        Serial.println();
    }
    
    delay(10);
}
