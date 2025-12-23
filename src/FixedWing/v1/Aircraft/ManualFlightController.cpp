#include "ManualFlightController.h"

ManualFlightController::ManualFlightController()
    : integrated_yaw(0.0f), yaw_rate(0.0f), last_update(0) {
    ctrl.target_roll = 0.0f;
    ctrl.target_pitch = 0.0f;
    ctrl.yaw_rate = 0.0f;
    ctrl.motor_speed = 0;
}

void ManualFlightController::processControl(const uint8_t* frame) {
    if(decodeUplinkControl(frame, &ctrl)) {
        last_update = millis();
    }
}

void ManualFlightController::execute(BLDCMotor* motor) {
    updateYaw();
    
    setTargetPitch(ctrl.target_pitch);
    setTargetRoll(ctrl.target_roll);
    setTargetYaw(integrated_yaw);
    yaw_rate = ctrl.yaw_rate;
    motor->setSpeed(ctrl.motor_speed);
}

void ManualFlightController::setRecoveryAttitude() {
    ctrl.target_pitch = -2.0f;
    ctrl.target_roll = 7.0f;
    yaw_rate = 5.0f;
    
    updateYaw();
    
    setTargetPitch(ctrl.target_pitch);
    setTargetRoll(ctrl.target_roll);
    setTargetYaw(integrated_yaw);
}

void ManualFlightController::updateYaw() {
    uint32_t now = millis();
    if(last_update == 0) {
        last_update = now;
        return;
    }
    
    float dt = (now - last_update) * 0.001f;
    
    integrated_yaw += yaw_rate * dt;
    if(integrated_yaw > 180.0f) integrated_yaw -= 360.0f;
    if(integrated_yaw < -180.0f) integrated_yaw += 360.0f;
    
    last_update = now;
}
