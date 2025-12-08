#ifndef LORA_RADIO_H
#define LORA_RADIO_H

#include <SPI.h>
#include <LoRa.h>

#define LORA_SS_PIN  5
#define LORA_RST_PIN 32
#define LORA_DIO0_PIN 26
#define LORA_SCK_PIN 18
#define LORA_MISO_PIN 19
#define LORA_MOSI_PIN 23

#define LORA_FREQUENCY 868E6
#define LORA_BANDWIDTH 125E3
#define LORA_SPREADING_FACTOR 7
#define LORA_SYNC_WORD 0xF3
#define LORA_TX_POWER 20
#define LORA_CODING_RATE 5

struct LoRaConfig {
    uint8_t ss_pin;
    uint8_t rst_pin;
    uint8_t dio0_pin;
    uint8_t sck_pin;
    uint8_t miso_pin;
    uint8_t mosi_pin;
    uint32_t frequency;
    uint32_t bandwidth;
    uint8_t spreading_factor;
    uint8_t sync_word;
    uint8_t tx_power;
    uint8_t coding_rate;
};

class LoRaRadio {
public:
    LoRaRadio();
    
    bool init();
    void sendFrame(const uint8_t* frame, size_t size);
    size_t receiveFrame(uint8_t* buffer, size_t max_size);
    
private:
    LoRaConfig config;
};

#endif
