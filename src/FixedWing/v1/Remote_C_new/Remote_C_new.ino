#include "MPU6050_Dual.h"
#include "LoRaRadio.h"
#include "FrameProtocol.h"

LoRaRadio lora;
UplinkControl ctrl;
uint8_t tx_frame[FRAME_SIZE];
uint32_t last_send_time = 0;

void setup() {
    Serial.begin(115200);
    
    initMPU6050Dual();
    Serial.println("MPU6050 initialized on Core 0");
    
    if(!lora.init()) {
        Serial.println("LoRa init failed");
        while(1);
    }
    Serial.println("LoRa initialized");
    
    last_send_time = millis();
    delay(1000);
}

void loop() {
    if(millis() - last_send_time >= 110) {
        float roll, pitch, yaw;
        getRollPitchYawSmoothed(&roll, &pitch, &yaw);
        
        ctrl.target_roll = roll / 1.8f;
        ctrl.target_pitch = pitch / 1.8f;
        ctrl.yaw_rate = roll / 1.8f * 0.5f;
        
        int Analog = analogRead(A0);
        ctrl.motor_speed = (uint16_t)((double)(1000 * Analog / 4096));
        
        encodeUplinkControl(tx_frame, &ctrl);
        lora.sendFrame(tx_frame, FRAME_SIZE);
        
        Serial.print("Roll: ");
        Serial.print(roll);
        Serial.print(" Pitch: ");
        Serial.print(pitch);
        Serial.print(" Yaw: ");
        Serial.println(yaw);
        
        last_send_time = millis();
    }
}
