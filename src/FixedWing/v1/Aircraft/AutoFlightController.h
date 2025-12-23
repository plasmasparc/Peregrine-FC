#ifndef AUTO_FLIGHT_CONTROLLER_H
#define AUTO_FLIGHT_CONTROLLER_H

#include "BLDCMotor.h"
#include "GuidanceController.h"
#include "GPS_Manager.h"
#include "IMU_Manager.h"
#include "PID_Controller.h"
#include <Arduino.h>

class AutoFlightController {
public:
    AutoFlightController();
    
    void execute(BLDCMotor* motor);
    
    GuidanceController* getGuidance() { return &guidance; }
    
private:
    GuidanceController guidance;
    uint16_t motor_speed;
};

#endif
