#ifndef MODULE_LOADER_H
#define MODULE_LOADER_H

#include "MPU6050.h"
#include "LoRaRadio.h"
#include "IMU_Manager.h"
#include "GPS_Manager.h"
#include "PID_Controller.h"
#include "BLDCMotor.h"

class ModuleLoader {
public:
    static bool initIMU();
    static bool initGPS();
    static bool initPID();
    static bool initLoRa(LoRaRadio* lora);
    static bool initMotor(BLDCMotor* motor);
    static bool initAll(LoRaRadio* lora, BLDCMotor* motor);
};

#endif