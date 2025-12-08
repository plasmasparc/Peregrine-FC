#include "LoRaRadio.h"
#include "FrameProtocol.h"
#include "DisplayManager.h"

LoRaRadio lora;
DisplayManager display_mgr;
uint8_t rx_buffer[FRAME_SIZE];
DownlinkTelemetry telem;
UplinkControl ctrl;
uint32_t last_packet_time = 0;
uint32_t downlink_count = 0;
uint32_t uplink_count = 0;
uint32_t error_count = 0;

void processDownlink() {
    if(decodeDownlinkTelem(rx_buffer, &telem)) {
        downlink_count++;
        last_packet_time = millis();
        
        Serial.println("--- DOWNLINK TELEM ---");
        Serial.print("Roll: "); Serial.println(telem.roll);
        Serial.print("Pitch: "); Serial.println(telem.pitch);
        Serial.print("Yaw: "); Serial.println(telem.yaw);
        Serial.print("Lat: "); Serial.println(telem.lat, 7);
        Serial.print("Lon: "); Serial.println(telem.lon, 7);
        Serial.print("Alt: "); Serial.println(telem.alt);
        Serial.print("Speed: "); Serial.println(telem.speed);
        Serial.print("Sats: "); Serial.println(telem.satellites);
        Serial.print("Fix: "); Serial.println(telem.fix);
        
        display_mgr.showDownlinkTelemetry(&telem, downlink_count, uplink_count, error_count);
    } else {
        error_count++;
    }
}

void processUplink() {
    if(decodeUplinkControl(rx_buffer, &ctrl)) {
        uplink_count++;
        last_packet_time = millis();
        
        Serial.println("--- UPLINK CONTROL ---");
        Serial.print("Target Roll: "); Serial.println(ctrl.target_roll);
        Serial.print("Target Pitch: "); Serial.println(ctrl.target_pitch);
        Serial.print("Yaw Rate: "); Serial.println(ctrl.yaw_rate);
        Serial.print("Motor Speed: "); Serial.println(ctrl.motor_speed);
        
        display_mgr.showUplinkControl(&ctrl, downlink_count, uplink_count, error_count);
    } else {
        error_count++;
    }
}

void setup() {
    Serial.begin(115200);
    
    if(!display_mgr.init()) {
        Serial.println("SSD1306 init failed");
        while(1);
    }
    
    display_mgr.showInitMessage("LoRa RX Init...");
    
    if(!lora.init()) {
        Serial.println("LoRa init failed");
        display_mgr.showInitMessage("LoRa FAIL");
        while(1);
    }
    
    Serial.println("LoRa RX ready");
    display_mgr.showInitMessage("LoRa RX Ready");
    delay(1000);
}

void loop() {
    size_t bytes_received = lora.receiveFrame(rx_buffer, FRAME_SIZE);
    
    if(bytes_received == FRAME_SIZE) {
        FrameType frame_type = identifyFrame(rx_buffer, bytes_received);
        
        switch(frame_type) {
            case FRAME_DOWNLINK:
                processDownlink();
                break;
            case FRAME_UPLINK:
                processUplink();
                break;
            case FRAME_ERROR:
                error_count++;
                Serial.println("Frame ERROR");
                break;
            case FRAME_NONE:
                break;
        }
    }
    
    if(millis() - last_packet_time > 5000 && (downlink_count > 0 || uplink_count > 0)) {
        display_mgr.showNoSignal((millis() - last_packet_time) / 1000);
    }
    
    delay(10);
}
