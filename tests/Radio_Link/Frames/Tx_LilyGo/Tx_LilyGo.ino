#include "LoRaRadio.h"
#include "FrameProtocol.h"

LoRaRadio lora;

void setup() {
    Serial.begin(115200);

    randomSeed(analogRead(0));
    
    // Initialize LoRa with default config
    if (lora.init()) {
        Serial.println("LoRa initialized");
    } else {
        Serial.println("LoRa init failed!");
        while(1);
    }
}

void loop() {
    DownlinkTelemetry telem_in;
    telem_in = generateDownlinkTelemetry();

    uint8_t frame[FRAME_SIZE];
    encodeDownlinkTelem(frame, &telem_in);

    lora.sendFrame(frame, FRAME_SIZE);
    
    delay(100);
    
    UplinkControl ctrl_in; 
    ctrl_in = generateUplinkControl();
    
    encodeUplinkControl(frame, &ctrl_in);

    lora.sendFrame(frame, FRAME_SIZE);
    
    delay(100);
    
}

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

DownlinkTelemetry generateDownlinkTelemetry() {
    
    Serial.println("=== Downlink Telemetry Generation ===");
    
    DownlinkTelemetry telem_in;
    
    // Szögadatok
    telem_in.roll = random(-900, 900) / 10.0f;
    telem_in.pitch = random(-900, 900) / 10.0f;
    telem_in.yaw = random(0, 3600) / 10.0f;
    
    // GPS koordináták
    telem_in.lat = random(-900000000, 900000000) / 1e7;
    telem_in.lon = random(-1800000000, 1800000000) / 1e7;
    
    // Magasság és sebesség
    telem_in.alt = random(-1000, 10000) / 10.0f;
    telem_in.speed = random(0, 10000) / 100.0f;
    
    // GPS státusz
    telem_in.satellites = random(0, 20);
    telem_in.fix = random(0, 2);
    
    Serial.println("Generated Telemetry Data:");
    printDownlinkTelemetry(&telem_in);
    
    return telem_in;
}

UplinkControl generateUplinkControl() {
    
    Serial.println("=== Uplink Control Generation ===");
    
    UplinkControl ctrl_in;
    
    // Cél szögadatok (roll/pitch: -90.0 és 90.0 fok között)
    ctrl_in.target_roll = random(-900, 900) / 10.0f;
    ctrl_in.target_pitch = random(-900, 900) / 10.0f;
    
    // Yaw forgási sebesség (pl. -90.00 és 90.00 deg/s között)
    ctrl_in.yaw_rate = random(-9000, 9000) / 100.0f;
    
    // Motor sebesség (0 és 1000 között)
    ctrl_in.motor_speed = random(0, 1001);
    
    Serial.println("Generated Control Data:");
    printUplinkControl(&ctrl_in);
    
    return ctrl_in;
}
