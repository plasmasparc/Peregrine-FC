#ifndef LORA_RADIO_H
#define LORA_RADIO_H

#include <SPI.h>
#include <LoRa.h>

// Default pin configuration for RP2040
#define LORA_RX_PIN  4
#define LORA_TX_PIN  3
#define LORA_SCK_PIN 2
#define LORA_SS_PIN  6
#define LORA_RST_PIN 7
#define LORA_DIO0_PIN 5

// Default RF parameters
#define LORA_FREQUENCY 868E6     // 868 MHz (EU band)
#define LORA_BANDWIDTH 125E3     // 125 kHz
#define LORA_SPREADING_FACTOR 7  // SF7
#define LORA_SYNC_WORD 0xF3
#define LORA_TX_POWER 20         // 20 dBm (max for SX1276)
#define LORA_CODING_RATE 5       // 4/5

// LoRa configuration structure
struct LoRaConfig {
    uint8_t rx_pin;
    uint8_t tx_pin;
    uint8_t sck_pin;
    uint8_t ss_pin;
    uint8_t rst_pin;
    uint8_t dio0_pin;
    uint32_t frequency;
    uint32_t bandwidth;
    uint8_t spreading_factor;
    uint8_t sync_word;
    uint8_t tx_power;
    uint8_t coding_rate;
};

// Received message structure
struct LoRaMessage {
    String data;
    int rssi;
    float snr;
    uint32_t timestamp;
};

class LoRaRadio {
public:
    LoRaRadio();
    LoRaRadio(const LoRaConfig& config);
    
    bool init();
    void send(const String& message);
    bool receive();
    
    // Message access
    bool hasMessage() const { return message_available; }
    const LoRaMessage& getMessage() const { return received_message; }
    void clearMessage() { message_available = false; }
    
    // Statistics
    int getRSSI() const { return received_message.rssi; }
    float getSNR() const { return received_message.snr; }
    uint32_t getLastSendTime() const { return last_send_time; }
    uint32_t getLastReceiveTime() const { return last_receive_time; }
    
    // Airtime calculation
    float calculateAirtime(size_t payload_bytes) const;
    
private:
    LoRaConfig config;
    LoRaMessage received_message;
    bool message_available;
    uint32_t last_send_time;
    uint32_t last_receive_time;
};

#endif
