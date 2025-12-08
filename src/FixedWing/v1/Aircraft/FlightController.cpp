#include "FlightController.h"

FlightController::FlightController(LoRaRadio* lora, BLDCMotor* motor)
    : lora(lora), motor(motor), last_rx_time(0), last_failsafe_tx(0),
      last_update_time(0), failsafe_mode(false), integrated_yaw(0.0f), yaw_rate(0.0f) {}

void FlightController::update() {
    uint32_t now = millis();
    
    if(last_update_time == 0) {
        last_update_time = now;
        return;
    }
    
    handleUplink();
    handleFailsafe();
    updateYaw();
    
    last_update_time = now;
}

void FlightController::handleUplink() {
    size_t bytes = lora->receiveFrame(rx_frame, FRAME_SIZE);
    
    if(bytes == FRAME_SIZE && identifyFrame(rx_frame, FRAME_SIZE) == FRAME_UPLINK) {
        if(decodeUplinkControl(rx_frame, &ctrl)) {
            last_rx_time = millis();
            failsafe_mode = false;
            
            setTargetPitch(ctrl.target_pitch);
            setTargetRoll(ctrl.target_roll);
            setTargetYaw(integrated_yaw);
            yaw_rate = ctrl.yaw_rate;
            motor->setSpeed(ctrl.motor_speed);
            
            buildTelemetry(&telem);
            encodeDownlinkTelem(tx_frame, &telem);
        }
    }
}

void FlightController::handleFailsafe() {
    if(millis() - last_rx_time > 500) {
        if(!failsafe_mode) {
            failsafe_mode = true;
            motor->stop();
            last_failsafe_tx = millis();
        }
        
        if(millis() - last_failsafe_tx >= 111) {
            buildTelemetry(&telem);
            encodeDownlinkTelem(tx_frame, &telem);
            lora->sendFrame(tx_frame, FRAME_SIZE);
            last_failsafe_tx = millis();
        }
        
        setTargetPitch(-2.0f);
        setTargetRoll(7.0f);
        setTargetYaw(integrated_yaw);
        yaw_rate = 5.0f;
    }
}

void FlightController::updateYaw() {
    uint32_t now = millis();
    float dt = (now - last_update_time) * 0.001f;
    
    integrated_yaw += yaw_rate * dt;
    if(integrated_yaw > 180.0f) integrated_yaw -= 360.0f;
    if(integrated_yaw < -180.0f) integrated_yaw += 360.0f;
}

void FlightController::buildTelemetry(DownlinkTelemetry* telem) {
    double lat, lon;
    float alt, speed;
    uint8_t hour, minute, seconds, satellites;
    bool fix;
    
    getAllGPSData(&lat, &lon, &alt, &speed, &hour, &minute, &seconds, &satellites, &fix);
    
    float roll, pitch, yaw;
    getAttitude(&roll, &pitch, &yaw);
    
    telem->roll = roll;
    telem->pitch = pitch;
    telem->yaw = yaw;
    telem->lat = lat;
    telem->lon = lon;
    telem->alt = alt;
    telem->speed = speed;
    telem->satellites = satellites;
    telem->fix = fix;
}