#include "ModuleLoader.h"
#include "FlightController.h"
#include "LoRaRadio.h"
#include "BLDCMotor.h"

LoRaRadio lora;
#define MOTOR_PIN 21
BLDCMotor motor(MOTOR_PIN, 1000, 2000);

FlightController fc(&lora, &motor);

uint32_t last_loop_time = 0;

void setup() {
    Serial.begin(115200);
    
    if(!ModuleLoader::initAll(&lora, &motor)) {
        Serial.println("Init failed");
        while(1);
    }
    
    last_loop_time = millis();
}

void loop() {
    
    if(millis() - last_loop_time >= 20) {
        fc.update();
        last_loop_time = millis();
    }
}