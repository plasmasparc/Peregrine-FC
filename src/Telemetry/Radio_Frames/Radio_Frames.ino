#include "FrameProtocol.h"

void printDownlinkTelemetry(const DownlinkTelemetry* data) {
    Serial.print("Roll: "); Serial.print(data->roll);
    Serial.print(" Pitch: "); Serial.print(data->pitch);
    Serial.print(" Yaw: "); Serial.println(data->yaw);
    Serial.print("Lat: "); Serial.print(data->lat, 7);
    Serial.print(" Lon: "); Serial.println(data->lon, 7);
    Serial.print("Alt: "); Serial.print(data->alt);
    Serial.print(" Speed: "); Serial.println(data->speed);
    Serial.print("Satellites: "); Serial.print(data->satellites);
    Serial.print(" Fix: "); Serial.println(data->fix);
}

void printUplinkControl(const UplinkControl* data) {
    Serial.print("Target Roll: "); Serial.print(data->target_roll);
    Serial.print(" Target Pitch: "); Serial.println(data->target_pitch);
    Serial.print("Yaw Rate: "); Serial.print(data->yaw_rate);
    Serial.print(" Motor Speed: "); Serial.println(data->motor_speed);
}

void printFrame(const uint8_t* frame) {
    for (size_t i = 0; i < FRAME_SIZE; i++) {
        if (frame[i] < 0x10) Serial.print("0");
        Serial.print(frame[i], HEX);
        Serial.print(" ");
    }
    Serial.println();
}

void setup() {
    Serial.begin(115200);
    while (!Serial);
    
    randomSeed(analogRead(0));
    
    Serial.println("=== Downlink Telemetry Test ===");
    
    DownlinkTelemetry telem_in;
    telem_in.roll = random(-900, 900) / 10.0f;
    telem_in.pitch = random(-900, 900) / 10.0f;
    telem_in.yaw = random(0, 3600) / 10.0f;
    telem_in.lat = random(-900000000, 900000000) / 1e7;
    telem_in.lon = random(-1800000000, 1800000000) / 1e7;
    telem_in.alt = random(-1000, 10000) / 10.0f;
    telem_in.speed = random(0, 10000) / 100.0f;
    telem_in.satellites = random(0, 20);
    telem_in.fix = random(0, 2);
    
    Serial.println("Input:");
    printDownlinkTelemetry(&telem_in);
    
    uint8_t frame[FRAME_SIZE];
    encodeDownlinkTelem(frame, &telem_in);
    
    Serial.print("Encoded: ");
    printFrame(frame);
    
    DownlinkTelemetry telem_out;
    bool result = decodeDownlinkTelem(frame, &telem_out);
    
    Serial.print("Decode result: ");
    Serial.println(result ? "SUCCESS" : "FAIL");
    Serial.println("Output:");
    printDownlinkTelemetry(&telem_out);
    
    Serial.println("\n=== Uplink Control Test ===");
    
    UplinkControl ctrl_in;
    ctrl_in.target_roll = random(-900, 900) / 10.0f;
    ctrl_in.target_pitch = random(-900, 900) / 10.0f;
    ctrl_in.yaw_rate = random(-9000, 9000) / 100.0f;
    ctrl_in.motor_speed = random(0, 1001);
    
    Serial.println("Input:");
    printUplinkControl(&ctrl_in);
    
    encodeUplinkControl(frame, &ctrl_in);
    
    Serial.print("Encoded: ");
    printFrame(frame);
    
    UplinkControl ctrl_out;
    result = decodeUplinkControl(frame, &ctrl_out);
    
    Serial.print("Decode result: ");
    Serial.println(result ? "SUCCESS" : "FAIL");
    Serial.println("Output:");
    printUplinkControl(&ctrl_out);
    
    Serial.println("\n=== 1-bit Error Test - Downlink ===");
    
    encodeDownlinkTelem(frame, &telem_in);
    
    uint8_t bit_pos = random(0, FRAME_SIZE * 8);
    uint8_t byte_pos = bit_pos / 8;
    uint8_t bit_offset = bit_pos % 8;
    frame[byte_pos] ^= (1 << bit_offset);
    
    Serial.print("Error injected at byte ");
    Serial.print(byte_pos);
    Serial.print(" bit ");
    Serial.println(bit_offset);
    Serial.print("Corrupted: ");
    printFrame(frame);
    
    result = decodeDownlinkTelem(frame, &telem_out);
    Serial.print("Decode result: ");
    Serial.println(result ? "SUCCESS" : "FAIL");
    
    Serial.println("\n=== 1-bit Error Test - Uplink ===");
    
    encodeUplinkControl(frame, &ctrl_in);
    
    bit_pos = random(0, FRAME_SIZE * 8);
    byte_pos = bit_pos / 8;
    bit_offset = bit_pos % 8;
    frame[byte_pos] ^= (1 << bit_offset);
    
    Serial.print("Error injected at byte ");
    Serial.print(byte_pos);
    Serial.print(" bit ");
    Serial.println(bit_offset);
    Serial.print("Corrupted: ");
    printFrame(frame);
    
    result = decodeUplinkControl(frame, &ctrl_out);
    Serial.print("Decode result: ");
    Serial.println(result ? "SUCCESS" : "FAIL");
}

void loop() {
}
