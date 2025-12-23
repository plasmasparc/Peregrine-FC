#include "LoRaRadio.h"
#include "FrameProtocol.h"
#include "DisplayManager.h"
#include "WebTracker.h"

#define DISPLAY_DOWNLINK_ONLY 1

LoRaConfig lora_config;
LoRaRadio radio;
DisplayManager display;
WebTracker tracker;

uint32_t downlink_count = 0;
uint32_t uplink_count = 0;
uint32_t error_count = 0;
uint32_t last_receive_ms = 0;

DownlinkTelemetry last_telem;
UplinkControl last_ctrl;

void setup() {
    Serial.begin(115200);
    
    if(!display.init()) {
        Serial.println("Display init failed");
        while(1);
    }
    display.showInitMessage("Initializing...");
    lora_config.rx_pin = LORA_RX_PIN;
    lora_config.tx_pin = LORA_TX_PIN;
    lora_config.sck_pin = LORA_SCK_PIN;
    lora_config.ss_pin = LORA_SS_PIN;
    lora_config.rst_pin = LORA_RST_PIN;
    lora_config.dio0_pin = LORA_DIO0_PIN;
    lora_config.frequency = 433.92E6;
    lora_config.bandwidth = LORA_BANDWIDTH;
    lora_config.spreading_factor = LORA_SPREADING_FACTOR;
    lora_config.sync_word = LORA_SYNC_WORD;
    lora_config.tx_power = LORA_TX_POWER;
    lora_config.coding_rate = LORA_CODING_RATE;
    
    LoRaRadio radio_init(lora_config);
    radio = radio_init;
    
    if(!radio.init()) {
        Serial.println("LoRa init failed");
        display.showInitMessage("LoRa FAIL");
        while(1);
    }
    
    tracker.init();
    
    display.showInitMessage("Ready");
    delay(1000);
}

void loop() {
    uint8_t rx_buffer[FRAME_SIZE];
    size_t rx_size = radio.receiveFrame(rx_buffer, FRAME_SIZE);
    
    if(rx_size > 0) {
        FrameType type = identifyFrame(rx_buffer, rx_size);
        
        if(type == FRAME_DOWNLINK) {
            if(decodeDownlinkTelem(rx_buffer, &last_telem)) {
                downlink_count++;
                last_receive_ms = millis();
                tracker.addPosition(last_telem.lat, last_telem.lon, last_telem.roll, last_telem.pitch);
                
                #if DISPLAY_DOWNLINK_ONLY
                display.showDownlinkTelemetry(&last_telem, downlink_count, uplink_count, error_count, tracker.getIPAddress().c_str());
                #endif
            } else {
                error_count++;
            }
        } else if(type == FRAME_UPLINK) {
            if(decodeUplinkControl(rx_buffer, &last_ctrl)) {
                uplink_count++;
                last_receive_ms = millis();
                
                #if !DISPLAY_DOWNLINK_ONLY
                display.showUplinkControl(&last_ctrl, downlink_count, uplink_count, error_count);
                #endif
            } else {
                error_count++;
            }
        } else {
            error_count++;
        }
    }
    
    if(millis() - last_receive_ms > 5000 && last_receive_ms > 0) {
        uint32_t seconds_since = (millis() - last_receive_ms) / 1000;
        display.showNoSignal(seconds_since);
    }
    
    tracker.handleClient();
}
