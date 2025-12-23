#ifndef MANUAL_FLIGHT_CONTROLLER_H
#define MANUAL_FLIGHT_CONTROLLER_H

#include "BLDCMotor.h"
#include "FrameProtocol.h"
#include "PID_Controller.h"
#include <Arduino.h>

class ManualFlightController {
public:
    ManualFlightController();
    
    void processControl(const uint8_t* frame);
    void execute(BLDCMotor* motor);
    void setRecoveryAttitude();
    
private:
    UplinkControl ctrl;
    float integrated_yaw;
    float yaw_rate;
    uint32_t last_update;
    
    void updateYaw();
};

#endif
