#ifndef FLIGHT_CONTROLLER_H
#define FLIGHT_CONTROLLER_H

#include "LoRaRadio.h"
#include "FrameProtocol.h"
#include "PID_Controller.h"
#include "BLDCMotor.h"
#include "IMU_Manager.h"
#include "GPS_Manager.h"

class FlightController {
public:
    FlightController(LoRaRadio* lora, BLDCMotor* motor);
    
    void update();
    bool isFailsafe() const { return failsafe_mode; }
    
private:
    void handleUplink();
    void handleFailsafe();
    void updateYaw();
    void buildTelemetry(DownlinkTelemetry* telem);
    
    LoRaRadio* lora;
    BLDCMotor* motor;
    
    UplinkControl ctrl;
    DownlinkTelemetry telem;
    uint8_t rx_frame[FRAME_SIZE];
    uint8_t tx_frame[FRAME_SIZE];
    
    uint32_t last_rx_time;
    uint32_t last_failsafe_tx;
    uint32_t last_update_time;
    bool failsafe_mode;
    float integrated_yaw;
    float yaw_rate;
};
#
#endif
