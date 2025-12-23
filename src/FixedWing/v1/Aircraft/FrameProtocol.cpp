#include "FrameProtocol.h"

uint8_t crc8(const uint8_t* data, size_t len) {
    uint8_t crc = 0xFF;
    for (size_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; j++) {
            crc = (crc & 0x80) ? (crc << 1) ^ 0x07 : crc << 1;
        }
    }
    return crc;
}

uint16_t calculateValidity(const uint8_t* frame) {
    uint16_t sum = 0;
    for (size_t i = 2; i < FRAME_SIZE; i++) {
        sum += frame[i];
    }
    return sum;
}

FrameType identifyFrame(const uint8_t* frame, size_t size) {
    if(size == CMD_FRAME_SIZE) {
        if(identifyCommand(frame) != CMD_ERROR) {
            return FRAME_COMMAND;
        }
        return FRAME_ERROR;
    }
    
    if(size != FRAME_SIZE) return FRAME_ERROR;
    
    uint16_t validity = (frame[0] << 8) | frame[1];
    if(validity != calculateValidity(frame)) return FRAME_ERROR;
    
    uint8_t frame_id = frame[2];
    
    if(frame_id == FRAME_ID_DOWNLINK_TELEM) {
        if(crc8(&frame[4], FRAME_SIZE - 4) == frame[3]) {
            return FRAME_DOWNLINK;
        }
    } else if(frame_id == FRAME_ID_UPLINK_CONTROL) {
        if(crc8(&frame[4], FRAME_SIZE - 4) == frame[3]) {
            return FRAME_UPLINK;
        }
    }
    
    return FRAME_ERROR;
}

CommandType identifyCommand(const uint8_t* cmd_frame) {
    uint8_t crc_calc = crc8(cmd_frame, 4);
    if(crc_calc != cmd_frame[4]) return CMD_ERROR;
    
    uint8_t checksum = cmd_frame[0] + cmd_frame[1] + cmd_frame[2] + cmd_frame[3];
    if(checksum != cmd_frame[5]) return CMD_ERROR;
    
    uint8_t cmd_id = cmd_frame[0];
    
    switch(cmd_id) {
        case CMD_ID_LOITER_MODE:     return CMD_LOITER_MODE;
        case CMD_ID_WAYPOINT_MODE:   return CMD_WAYPOINT_MODE;
        case CMD_ID_MANUAL_MODE:     return CMD_MANUAL_MODE;
        case CMD_ID_ADD_WAYPOINT:    return CMD_ADD_WAYPOINT;
        case CMD_ID_DELETE_WAYPOINT: return CMD_DELETE_WAYPOINT;
        case CMD_ID_GET_WAYPOINT:    return CMD_GET_WAYPOINT;
        case CMD_ID_CLEAR_MISSION:   return CMD_CLEAR_MISSION;
        default: return CMD_ERROR;
    }
}

void encodeDownlinkTelem(uint8_t* frame, const DownlinkTelemetry* data) {
    memset(frame, 0, FRAME_SIZE);
    
    frame[2] = FRAME_ID_DOWNLINK_TELEM;
    
    int16_t roll = (int16_t)(data->roll * 10);
    int16_t pitch = (int16_t)(data->pitch * 10);
    int16_t yaw = (int16_t)(data->yaw * 10);
    
    frame[4] = (roll >> 8) & 0xFF;
    frame[5] = roll & 0xFF;
    frame[6] = (pitch >> 8) & 0xFF;
    frame[7] = pitch & 0xFF;
    frame[8] = (yaw >> 8) & 0xFF;
    frame[9] = yaw & 0xFF;
    
    int32_t lat = (int32_t)(data->lat * 1e7);
    int32_t lon = (int32_t)(data->lon * 1e7);
    int16_t alt = (int16_t)(data->alt * 10);
    uint16_t speed = (uint16_t)(data->speed * 100);
    
    frame[10] = (lat >> 24) & 0xFF;
    frame[11] = (lat >> 16) & 0xFF;
    frame[12] = (lat >> 8) & 0xFF;
    frame[13] = lat & 0xFF;
    frame[14] = (lon >> 24) & 0xFF;
    frame[15] = (lon >> 16) & 0xFF;
    frame[16] = (lon >> 8) & 0xFF;
    frame[17] = lon & 0xFF;
    frame[18] = (alt >> 8) & 0xFF;
    frame[19] = alt & 0xFF;
    frame[20] = (speed >> 8) & 0xFF;
    frame[21] = speed & 0xFF;
    frame[22] = data->satellites;
    frame[23] = data->fix ? 0x01 : 0x00;
    
    frame[3] = crc8(&frame[4], FRAME_SIZE - 4);
    
    uint16_t validity = calculateValidity(frame);
    frame[0] = (validity >> 8) & 0xFF;
    frame[1] = validity & 0xFF;
}

bool decodeDownlinkTelem(const uint8_t* frame, DownlinkTelemetry* data) {
    uint16_t validity = (frame[0] << 8) | frame[1];
    if (validity != calculateValidity(frame)) return false;
    if (frame[2] != FRAME_ID_DOWNLINK_TELEM) return false;
    if (crc8(&frame[4], FRAME_SIZE - 4) != frame[3]) return false;
    
    int16_t roll = (frame[4] << 8) | frame[5];
    int16_t pitch = (frame[6] << 8) | frame[7];
    int16_t yaw = (frame[8] << 8) | frame[9];
    
    data->roll = roll / 10.0f;
    data->pitch = pitch / 10.0f;
    data->yaw = yaw / 10.0f;
    
    int32_t lat = ((int32_t)frame[10] << 24) | ((int32_t)frame[11] << 16) | ((int32_t)frame[12] << 8) | frame[13];
    int32_t lon = ((int32_t)frame[14] << 24) | ((int32_t)frame[15] << 16) | ((int32_t)frame[16] << 8) | frame[17];
    int16_t alt = (frame[18] << 8) | frame[19];
    uint16_t speed = (frame[20] << 8) | frame[21];
    
    data->lat = lat / 1e7;
    data->lon = lon / 1e7;
    data->alt = alt / 10.0f;
    data->speed = speed / 100.0f;
    data->satellites = frame[22];
    data->fix = (frame[23] & 0x01) != 0;
    
    return true;
}

void encodeUplinkControl(uint8_t* frame, const UplinkControl* data) {
    memset(frame, 0, FRAME_SIZE);
    
    frame[2] = FRAME_ID_UPLINK_CONTROL;
    
    int16_t target_roll = (int16_t)(data->target_roll * 100);
    int16_t target_pitch = (int16_t)(data->target_pitch * 100);
    int16_t yaw_rate = (int16_t)(data->yaw_rate * 100);
    
    frame[4] = (target_roll >> 8) & 0xFF;
    frame[5] = target_roll & 0xFF;
    frame[6] = (target_pitch >> 8) & 0xFF;
    frame[7] = target_pitch & 0xFF;
    frame[8] = (yaw_rate >> 8) & 0xFF;
    frame[9] = yaw_rate & 0xFF;
    frame[10] = (data->motor_speed >> 8) & 0xFF;
    frame[11] = data->motor_speed & 0xFF;
    
    frame[3] = crc8(&frame[4], FRAME_SIZE - 4);
    
    uint16_t validity = calculateValidity(frame);
    frame[0] = (validity >> 8) & 0xFF;
    frame[1] = validity & 0xFF;
}

bool decodeUplinkControl(const uint8_t* frame, UplinkControl* data) {
    uint16_t validity = (frame[0] << 8) | frame[1];
    if (validity != calculateValidity(frame)) return false;
    if (frame[2] != FRAME_ID_UPLINK_CONTROL) return false;
    if (crc8(&frame[4], FRAME_SIZE - 4) != frame[3]) return false;
    
    int16_t target_roll = (frame[4] << 8) | frame[5];
    int16_t target_pitch = (frame[6] << 8) | frame[7];
    int16_t yaw_rate = (frame[8] << 8) | frame[9];
    uint16_t motor_speed = (frame[10] << 8) | frame[11];
    
    data->target_roll = target_roll / 100.0f;
    data->target_pitch = target_pitch / 100.0f;
    data->yaw_rate = yaw_rate / 100.0f;
    data->motor_speed = motor_speed;
    
    return true;
}

void encodeCommand(uint8_t* cmd_frame, const CommandFrame* cmd) {
    cmd_frame[0] = cmd->cmd_id;
    cmd_frame[1] = cmd->param1;
    cmd_frame[2] = cmd->param2;
    cmd_frame[3] = cmd->param3;
    cmd_frame[4] = crc8(cmd_frame, 4);
    cmd_frame[5] = cmd_frame[0] + cmd_frame[1] + cmd_frame[2] + cmd_frame[3];
}

bool decodeCommand(const uint8_t* cmd_frame, CommandFrame* cmd) {
    uint8_t crc_calc = crc8(cmd_frame, 4);
    if(crc_calc != cmd_frame[4]) return false;
    
    uint8_t checksum = cmd_frame[0] + cmd_frame[1] + cmd_frame[2] + cmd_frame[3];
    if(checksum != cmd_frame[5]) return false;
    
    cmd->cmd_id = cmd_frame[0];
    cmd->param1 = cmd_frame[1];
    cmd->param2 = cmd_frame[2];
    cmd->param3 = cmd_frame[3];
    
    return true;
}
