#include "LoRaRadio.h"
#include "FrameProtocol.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

LoRaRadio lora;
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
        
        display.clearDisplay();
        display.setCursor(0,0);
        display.print("DL:");
        display.print(downlink_count);
        display.print(" UL:");
        display.print(uplink_count);
        display.print(" E:");
        display.println(error_count);
        display.print("R:");
        display.print(telem.roll,1);
        display.print(" P:");
        display.println(telem.pitch,1);
        display.print("Y:");
        display.println(telem.yaw,1);
        display.print("Alt:");
        display.print(telem.alt,1);
        display.print(" Spd:");
        display.println(telem.speed,1);
        display.print("Sats:");
        display.print(telem.satellites);
        display.print(" Fix:");
        display.println(telem.fix);
        display.display();
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
        
        display.clearDisplay();
        display.setCursor(0,0);
        display.print("DL:");
        display.print(downlink_count);
        display.print(" UL:");
        display.print(uplink_count);
        display.print(" E:");
        display.println(error_count);
        display.println("UPLINK CTRL");
        display.print("TgtR:");
        display.print(ctrl.target_roll,1);
        display.print(" TgtP:");
        display.println(ctrl.target_pitch,1);
        display.print("YawRate:");
        display.println(ctrl.yaw_rate,2);
        display.print("Motor:");
        display.println(ctrl.motor_speed);
        display.display();
    } else {
        error_count++;
    }
}

void setup() {
    Serial.begin(115200);
    
    if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
        Serial.println("SSD1306 init failed");
        while(1);
    }
    
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0,0);
    display.println("LoRa RX Init...");
    display.display();
    
    if(!lora.init()) {
        Serial.println("LoRa init failed");
        display.clearDisplay();
        display.setCursor(0,0);
        display.println("LoRa FAIL");
        display.display();
        while(1);
    }
    
    Serial.println("LoRa RX ready");
    display.clearDisplay();
    display.setCursor(0,0);
    display.println("LoRa RX Ready");
    display.display();
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
        display.clearDisplay();
        display.setCursor(0,0);
        display.println("NO SIGNAL");
        display.print("Last: ");
        display.print((millis() - last_packet_time)/1000);
        display.println("s ago");
        display.display();
    }
    delay(10);
}
