// FrameProtocol.h
#ifndef FRAME_PROTOCOL_H
#define FRAME_PROTOCOL_H

#include <Arduino.h>

#define SYNC_MARKER 0xAA55
#define FRAME_SIZE 32

// Frame IDs
#define FRAME_ID_DOWNLINK_TELEM_1 0xD1

// Frame structure for DOWNLINK_TELEMETRY_1
// [0-1]   0xAA55   - Sync marker
// [2]     0xD1     - Frame ID
// [3]     CRC8     - Error correction
// [4-5]   int16    - Roll (deg * 10)
// [6-7]   int16    - Pitch (deg * 10)
// [8-9]   uint16   - Yaw (deg * 10)
// [10-13] int32    - Latitude (deg * 1e7)
// [14-17] int32    - Longitude (deg * 1e7)
// [18-19] int16    - Altitude (m * 10)
// [20-21] uint16   - Speed (m/s * 100)
// [22]    uint8    - Satellites
// [23]    uint8    - Status flags (bit 0: GPS fix)
// [24-31] Reserved

struct TelemetryData {
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

uint8_t crc8(const uint8_t* data, size_t len);
void encodeDownlinkTelem1(uint8_t* frame, const TelemetryData* data);
bool decodeDownlinkTelem1(const uint8_t* frame, TelemetryData* data);

#endif
