#include "LoRaRadio.h"

LoRaRadio lora;

void setup() {
    // Initialize LoRa with default config
    if (lora.init()) {
        Serial.println("LoRa initialized");
    } else {
        Serial.println("LoRa init failed!");
        while(1);
    }
}

void loop() {
    // Build telemetry message
    String telemetry = "Test Attila";
    
    
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
