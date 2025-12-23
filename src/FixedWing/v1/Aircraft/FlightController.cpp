#include "FlightController.h"

FlightController::FlightController(LoRaRadio* lora, BLDCMotor* motor)
    : lora(lora), motor(motor), mode(MODE_MANUAL),
      last_rx_time(0), last_telemetry_tx(0) {}

void FlightController::update() {
    processUplink();
    checkFailsafe();
    executeMode();
    //if(mode_str=="FAILSAFE")
    //  sendTelemetry();
    
    const char* mode_str = (mode == MODE_MANUAL) ? "MANUAL" : 
                           (mode == MODE_AUTO) ? "AUTO" : "FAILSAFE";

    if(mode_str=="FAILSAFE")
      sendTelemetry();
    
    Serial.print("Mode: ");
    Serial.print(mode_str);
    Serial.print(" | Tgt R: ");
    Serial.print(manual_ctrl.getTargetRoll(), 1);
    Serial.print(" P: ");
    Serial.print(manual_ctrl.getTargetPitch(), 1);
    Serial.print(" Y: ");
    Serial.println(manual_ctrl.getIntegratedYaw(), 1);
}

void FlightController::processUplink() {
    size_t bytes = lora->receiveFrame(rx_frame, FRAME_SIZE);
    
    if(bytes == FRAME_SIZE) {
        FrameType type = identifyFrame(rx_frame, FRAME_SIZE);
        
        if(type == FRAME_UPLINK) {
            last_rx_time = millis();
            
            if(mode == MODE_FAILSAFE) {
                mode = MODE_MANUAL;
            }
            
            manual_ctrl.processControl(rx_frame);
        }
    }
}

void FlightController::executeMode() {
    switch(mode) {
        case MODE_MANUAL:
            manual_ctrl.execute(motor);
            break;
            
        case MODE_AUTO:
            auto_ctrl.execute(motor);
            break;
            
        case MODE_FAILSAFE:
            motor->stop();
            manual_ctrl.setRecoveryAttitude();
            break;
    }
}

void FlightController::checkFailsafe() {
    if(millis() - last_rx_time > 500 && last_rx_time != 0) {
        mode = MODE_FAILSAFE;
    }
}

void FlightController::sendTelemetry() {
    if(millis() - last_telemetry_tx >= 111) {
        buildTelemetry(&telem);
        encodeDownlinkTelem(tx_frame, &telem);
        lora->sendFrame(tx_frame, FRAME_SIZE);
        last_telemetry_tx = millis();
    }
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
