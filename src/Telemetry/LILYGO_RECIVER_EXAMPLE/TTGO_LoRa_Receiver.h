#ifndef T3_LORA_RECEIVER_H
#define T3_LORA_RECEIVER_H

#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// TTGO T3 V1.6.1 LoRa pinout (SX1276)
#define LORA_SCK  5
#define LORA_MISO 19
#define LORA_MOSI 27
#define LORA_CS   18
#define LORA_RST  23
#define LORA_DIO0 26

// OLED SSD1306 0.96" (I2C)
#define OLED_SDA 21
#define OLED_SCL 22
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// LoRa configuration
#define LORA_FREQUENCY 868E6
#define LORA_BANDWIDTH 125E3
#define LORA_SPREADING_FACTOR 7
#define LORA_SYNC_WORD 0xF3

struct TelemetryData {
    float roll;
    float pitch;
    float yaw;
    double latitude;
    double longitude;
    float altitude;
    float speed;
    uint8_t satellites;
    bool gps_fix;
    int rssi;
    float snr;
    uint32_t timestamp;
    bool valid;
};

class T3LoRaReceiver {
public:
    bool init();
    bool receive();
    const TelemetryData& getData() const { return data; }
    void displayOnOLED();
    
private:
    Adafruit_SSD1306 display;
    TelemetryData data;
    
    void initOLED();
    void initLoRa();
    bool parseMessage(String msg);
};

#endif
