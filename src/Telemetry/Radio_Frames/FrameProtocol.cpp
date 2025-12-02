// FrameProtocol.cpp
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

void encodeDownlinkTelem1(uint8_t* frame, const TelemetryData* data) {
    memset(frame, 0, FRAME_SIZE);
    
    // Sync marker
    frame[0] = (SYNC_MARKER >> 8) & 0xFF;
    frame[1] = SYNC_MARKER & 0xFF;
    
    // Frame ID
    frame[2] = FRAME_ID_DOWNLINK_TELEM_1;
    
    // IMU
    int16_t roll = (int16_t)(data->roll * 10);
    int16_t pitch = (int16_t)(data->pitch * 10);
    uint16_t yaw = (uint16_t)(data->yaw * 10);
    
    frame[4] = (roll >> 8) & 0xFF;
    frame[5] = roll & 0xFF;
    frame[6] = (pitch >> 8) & 0xFF;
    frame[7] = pitch & 0xFF;
    frame[8] = (yaw >> 8) & 0xFF;
    frame[9] = yaw & 0xFF;
    
    // GPS
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
    
    // CRC
    frame[3] = crc8(&frame[2], FRAME_SIZE - 2);
}

bool decodeDownlinkTelem1(const uint8_t* frame, TelemetryData* data) {
    // Verify sync
    uint16_t sync = (frame[0] << 8) | frame[1];
    if (sync != SYNC_MARKER) return false;
    
    // Verify frame ID
    if (frame[2] != FRAME_ID_DOWNLINK_TELEM_1) return false;
    
    // Verify CRC
    uint8_t crc = crc8(&frame[2], FRAME_SIZE - 2);
    if (crc != frame[3]) return false;
    
    // Decode IMU
    int16_t roll = (frame[4] << 8) | frame[5];
    int16_t pitch = (frame[6] << 8) | frame[7];
    uint16_t yaw = (frame[8] << 8) | frame[9];
    
    data->roll = roll / 10.0f;
    data->pitch = pitch / 10.0f;
    data->yaw = yaw / 10.0f;
    
    // Decode GPS
    int32_t lat = (frame[10] << 24) | (frame[11] << 16) | (frame[12] << 8) | frame[13];
    int32_t lon = (frame[14] << 24) | (frame[15] << 16) | (frame[16] << 8) | frame[17];
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
