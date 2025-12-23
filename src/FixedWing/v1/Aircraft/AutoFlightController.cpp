#include "AutoFlightController.h"

AutoFlightController::AutoFlightController() : motor_speed(0) {}

void AutoFlightController::execute(BLDCMotor* motor) {
    double lat, lon;
    float alt, speed;
    uint8_t hour, minute, seconds, satellites;
    bool fix;
    
    getAllGPSData(&lat, &lon, &alt, &speed, &hour, &minute, &seconds, &satellites, &fix);
    
    if(!fix) {
        motor->stop();
        return;
    }
    
    float lambda_rad = lon * DEG_TO_RAD;
    float phi_rad = lat * DEG_TO_RAD;
    
    float heading_cmd_rad = guidance.update(lambda_rad, phi_rad);
    float heading_cmd_deg = heading_cmd_rad * RAD_TO_DEG;
    
    setTargetPitch(0.0f);
    setTargetRoll(0.0f);
    setTargetYaw(heading_cmd_deg);
    
    motor->setSpeed(motor_speed);
}
