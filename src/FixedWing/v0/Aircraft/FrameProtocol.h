#ifndef FRAME_PROTOCOL_H
#define FRAME_PROTOCOL_H

#include <Arduino.h>

#define FRAME_SIZE 24

#define FRAME_ID_DOWNLINK_TELEM 0xD1
#define FRAME_ID_UPLINK_CONTROL 0xA1

enum FrameType {
    FRAME_NONE,
    FRAME_DOWNLINK,
    FRAME_UPLINK,
    FRAME_ERROR
};

struct DownlinkTelemetry {
    float roll;
    float pitch;
    float yaw;
    double lat;
    double lon;
    float alt;
    float speed;
    uint8_t satellites;
    bool fix;
};

struct UplinkControl {
    float target_roll;
    float target_pitch;
    float yaw_rate;
    uint16_t motor_speed;
};

uint8_t crc8(const uint8_t* data, size_t len);
uint16_t calculateValidity(const uint8_t* frame);
FrameType identifyFrame(const uint8_t* frame, size_t size);
void encodeDownlinkTelem(uint8_t* frame, const DownlinkTelemetry* data);
bool decodeDownlinkTelem(const uint8_t* frame, DownlinkTelemetry* data);
void encodeUplinkControl(uint8_t* frame, const UplinkControl* data);
bool decodeUplinkControl(const uint8_t* frame, UplinkControl* data);

#endif
