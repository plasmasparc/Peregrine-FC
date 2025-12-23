#include "LoRaRadio.h"
#include "FrameProtocol.h"
#include "NEO6_8.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

#define TX_INTERVAL_MS 150

LoRaConfig lora_config;
LoRaRadio radio;
NEO6_8 gps;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

uint32_t last_tx_ms = 0;
uint32_t packet_count = 0;

void setup() {
    Serial.begin(115200);
    Serial.println("GPS Backup 433.92MHz");
    
    if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
        Serial.println("Display FAILED");
    }
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("GPS Backup");
    display.display();
    
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
        Serial.println("LoRa FAILED");
        display.setCursor(0, 10);
        display.println("LoRa FAILED");
        display.display();
        while(1) delay(1000);
    }
    Serial.println("LoRa OK");
    
    gps.init();
    Serial.println("GPS OK");
    
    display.setCursor(0, 10);
    display.println("Ready");
    display.display();
    delay(1000);
}

void loop() {
    gps.update();
    
    if(millis() - last_tx_ms >= TX_INTERVAL_MS) {
        const GPSData& gps_data = gps.getData();
        
        DownlinkTelemetry telem;
        telem.roll = 0.0f;
        telem.pitch = 0.0f;
        telem.yaw = 0.0f;
        telem.lat = gps_data.latitude;
        telem.lon = gps_data.longitude;
        telem.alt = gps_data.altitude;
        telem.speed = gps_data.speed;
        telem.satellites = gps_data.satellites;
        telem.fix = gps_data.fix;
        
        uint8_t frame[FRAME_SIZE];
        encodeDownlinkTelem(frame, &telem);
        radio.sendFrame(frame, FRAME_SIZE);
        
        packet_count++;
        
        Serial.print("PKT:");
        Serial.print(packet_count);
        Serial.print(" LAT:");
        Serial.print(telem.lat, 7);
        Serial.print(" LON:");
        Serial.print(telem.lon, 7);
        Serial.print(" ALT:");
        Serial.print(telem.alt, 1);
        Serial.print(" SPD:");
        Serial.print(telem.speed, 2);
        Serial.print(" SAT:");
        Serial.print(telem.satellites);
        Serial.print(" FIX:");
        Serial.println(telem.fix);
        
        display.clearDisplay();
        display.setCursor(0, 0);
        display.print("PKT:");
        display.println(packet_count);
        display.print("LAT:");
        display.println(telem.lat, 7);
        display.print("LON:");
        display.println(telem.lon, 7);
        display.print("ALT:");
        display.print(telem.alt, 1);
        display.print(" SPD:");
        display.println(telem.speed, 1);
        display.print("SAT:");
        display.print(telem.satellites);
        display.print(" FIX:");
        display.println(telem.fix);
        display.display();
        
        last_tx_ms = millis();
    }
}
