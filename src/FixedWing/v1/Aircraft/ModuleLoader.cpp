#include "ModuleLoader.h"

bool ModuleLoader::initIMU() {
    initMPU6050();
    startIMU();
    return true;
}

bool ModuleLoader::initGPS() {
    startGPS();
    return true;
}

bool ModuleLoader::initPID() {
    startPID();
    setTargetRoll(0.0f);
    setTargetPitch(0.0f);
    setTargetYaw(0.0f);
    return true;
}

bool ModuleLoader::initLoRa(LoRaRadio* lora) {
    return lora->init();
}

bool ModuleLoader::initMotor(BLDCMotor* motor) {
    motor->init();
    return true;
}

bool ModuleLoader::initAll(LoRaRadio* lora, BLDCMotor* motor) {
    if(!initIMU()) return false;
    delay(100);
    
    if(!initGPS()) return false;
    delay(100);
    
    if(!initPID()) return false;
    delay(100);
    
    if(!initLoRa(lora)) return false;
    delay(100);
    
    if(!initMotor(motor)) return false;
    delay(100);
    
    return true;
}