#ifndef FRAME_PROTOCOL_H
#define FRAME_PROTOCOL_H

#include <Arduino.h>

#define FRAME_SIZE 24
#define CMD_FRAME_SIZE 6

// Frame IDs - 24 byte frames
#define FRAME_ID_DOWNLINK_TELEM 0xD1
#define FRAME_ID_UPLINK_CONTROL 0xA1

// Command Frame IDs - 6 byte frames
#define CMD_ID_LOITER_MODE      0xC1
#define CMD_ID_WAYPOINT_MODE    0xC2
#define CMD_ID_MANUAL_MODE      0xC3
#define CMD_ID_ADD_WAYPOINT     0xC4
#define CMD_ID_DELETE_WAYPOINT  0xC5
#define CMD_ID_GET_WAYPOINT     0xC6
#define CMD_ID_CLEAR_MISSION    0xC7

enum FrameType {
    FRAME_NONE,
    FRAME_DOWNLINK,
    FRAME_UPLINK,
    FRAME_COMMAND,
    FRAME_ERROR
};

enum CommandType {
    CMD_NONE,
    CMD_LOITER_MODE,
    CMD_WAYPOINT_MODE,
    CMD_MANUAL_MODE,
    CMD_ADD_WAYPOINT,
    CMD_DELETE_WAYPOINT,
    CMD_GET_WAYPOINT,
    CMD_CLEAR_MISSION,
    CMD_ERROR
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

struct CommandFrame {
    uint8_t cmd_id;
    uint8_t param1;
    uint8_t param2;
    uint8_t param3;
};

uint8_t crc8(const uint8_t* data, size_t len);
uint16_t calculateValidity(const uint8_t* frame);
FrameType identifyFrame(const uint8_t* frame, size_t size);
CommandType identifyCommand(const uint8_t* cmd_frame);

void encodeDownlinkTelem(uint8_t* frame, const DownlinkTelemetry* data);
bool decodeDownlinkTelem(const uint8_t* frame, DownlinkTelemetry* data);
void encodeUplinkControl(uint8_t* frame, const UplinkControl* data);
bool decodeUplinkControl(const uint8_t* frame, UplinkControl* data);

void encodeCommand(uint8_t* cmd_frame, const CommandFrame* cmd);
bool decodeCommand(const uint8_t* cmd_frame, CommandFrame* cmd);

#endif
