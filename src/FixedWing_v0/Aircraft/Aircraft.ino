#include "MPU6050.h"
#include "LoRaRadio.h"
#include "FrameProtocol.h"
#include "IMU_Manager.h"
#include "GPS_Manager.h"
#include "PID_Controller.h"
#include "BLDCMotor.h"

LoRaRadio lora;
UplinkControl ctrl;
DownlinkTelemetry telem;
uint8_t rx_frame[FRAME_SIZE];
uint8_t tx_frame[FRAME_SIZE];

#define MOTOR_PIN 21
BLDCMotor motor(MOTOR_PIN, 1000, 2000);

uint32_t last_rx_time = 0;
uint32_t last_loop_time = 0;
uint32_t last_failsafe_tx = 0;
bool failsafe_mode = false;
float integrated_yaw = 0.0f;
float yaw_rate = 0.0f;

void setup() {
    Serial.begin(115200);
    
    initMPU6050();
    Serial.println("MPU6050 initialized");
    
    startIMU();
    startGPS();
    delay(1000);
    
    startPID();
    setTargetRoll(0.0f);
    setTargetPitch(0.0f);
    setTargetYaw(0.0f);
    
    if(!lora.init()) {
        Serial.println("LoRa init failed");
        while(1);
    }
    Serial.println("LoRa initialized");
    
    motor.init();
    
    last_rx_time = millis();
    last_loop_time = millis();
    delay(1000);
}

void loop() {
    if(millis() - last_loop_time >= 20) {
        size_t bytes = lora.receiveFrame(rx_frame, FRAME_SIZE);
        
        if(bytes == FRAME_SIZE && identifyFrame(rx_frame, FRAME_SIZE) == FRAME_UPLINK) {
            if(decodeUplinkControl(rx_frame, &ctrl)) {
                last_rx_time = millis();
                failsafe_mode = false;
                
                setTargetPitch(ctrl.target_pitch);
                setTargetRoll(ctrl.target_roll);
                setTargetYaw(integrated_yaw);
                yaw_rate = ctrl.yaw_rate;
                motor.setSpeed(ctrl.motor_speed);
                
                buildTelemetry(&telem);
                encodeDownlinkTelem(tx_frame, &telem);
                //lora.sendFrame(tx_frame, FRAME_SIZE);
            }
        }
        
        if(millis() - last_rx_time > 500) {
            if(!failsafe_mode) {
                failsafe_mode = true;
                motor.stop();
                setTargetPitch(-1.0f);
                setTargetRoll(7.0f);
                yaw_rate = 2.0f;
                last_failsafe_tx = millis();
            }
            
            if(millis() - last_failsafe_tx >= 111) {
                buildTelemetry(&telem);
                encodeDownlinkTelem(tx_frame, &telem);
                lora.sendFrame(tx_frame, FRAME_SIZE);
                last_failsafe_tx = millis();
            }
        }
        
        integrated_yaw += yaw_rate * 0.02f;
        if(integrated_yaw > 180.0f) integrated_yaw -= 360.0f;
        if(integrated_yaw < -180.0f) integrated_yaw += 360.0f;
        
        last_loop_time = millis();
    }
}

void buildTelemetry(DownlinkTelemetry* telem) {
    getAttitude(&telem->roll, &telem->pitch, &telem->yaw);
    getAllGPSData(&telem->lat, &telem->lon, &telem->alt, &telem->speed, 
                  nullptr, nullptr, nullptr, &telem->satellites, &telem->fix);
}
