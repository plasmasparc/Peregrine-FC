#ifndef FLIGHT_CONTROLLER_H
#define FLIGHT_CONTROLLER_H

#include "ManualFlightController.h"
#include "AutoFlightController.h"
#include "LoRaRadio.h"
#include "BLDCMotor.h"
#include "FrameProtocol.h"
#include "GPS_Manager.h"
#include "IMU_Manager.h"

enum FlightMode {
    MODE_MANUAL,
    MODE_AUTO,
    MODE_FAILSAFE
};

class FlightController {
public:
    FlightController(LoRaRadio* lora, BLDCMotor* motor);
    
    void update();
    FlightMode getMode() const { return mode; }
    bool isFailsafe() const { return mode == MODE_FAILSAFE; }
    
private:
    void processUplink();
    void executeMode();
    void sendTelemetry();
    void checkFailsafe();
    void buildTelemetry(DownlinkTelemetry* telem);
    
    LoRaRadio* lora;
    BLDCMotor* motor;
    
    ManualFlightController manual_ctrl;
    AutoFlightController auto_ctrl;
    
    FlightMode mode;
    uint8_t rx_frame[FRAME_SIZE];
    uint8_t tx_frame[FRAME_SIZE];
    DownlinkTelemetry telem;
    
    uint32_t last_rx_time;
    uint32_t last_telemetry_tx;
};

#endif