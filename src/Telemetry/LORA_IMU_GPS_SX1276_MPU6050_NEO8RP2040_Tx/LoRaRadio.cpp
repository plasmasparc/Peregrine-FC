#include "LoRaRadio.h"
#include <math.h>

LoRaRadio::LoRaRadio() {
    // Default configuration
    config.rx_pin = LORA_RX_PIN;
    config.tx_pin = LORA_TX_PIN;
    config.sck_pin = LORA_SCK_PIN;
    config.ss_pin = LORA_SS_PIN;
    config.rst_pin = LORA_RST_PIN;
    config.dio0_pin = LORA_DIO0_PIN;
    config.frequency = LORA_FREQUENCY;
    config.bandwidth = LORA_BANDWIDTH;
    config.spreading_factor = LORA_SPREADING_FACTOR;
    config.sync_word = LORA_SYNC_WORD;
    config.tx_power = LORA_TX_POWER;
    config.coding_rate = LORA_CODING_RATE;
    
    message_available = false;
    last_send_time = 0;
    last_receive_time = 0;
}

LoRaRadio::LoRaRadio(const LoRaConfig& cfg) : config(cfg) {
    message_available = false;
    last_send_time = 0;
    last_receive_time = 0;
}

bool LoRaRadio::init() {
    // Configure SPI pins
    SPI.setRX(config.rx_pin);
    SPI.setTX(config.tx_pin);
    SPI.setSCK(config.sck_pin);
    SPI.begin();
    
    // Set LoRa pins
    LoRa.setPins(config.ss_pin, config.rst_pin, config.dio0_pin);
    
    // Initialize LoRa radio
    if (!LoRa.begin(config.frequency)) {
        return false;
    }
    
    // Configure radio parameters
    LoRa.setSignalBandwidth(config.bandwidth);
    LoRa.setSpreadingFactor(config.spreading_factor);
    LoRa.setSyncWord(config.sync_word);
    LoRa.setTxPower(config.tx_power, PA_OUTPUT_PA_BOOST_PIN);
    LoRa.setCodingRate4(config.coding_rate);
    
    return true;
}

void LoRaRadio::send(const String& message) {
    LoRa.beginPacket();
    LoRa.print(message);
    LoRa.endPacket();
    
    last_send_time = millis();
}

bool LoRaRadio::receive() {
    int packet_size = LoRa.parsePacket();
    
    if (packet_size > 0) {
        // Read packet data
        received_message.data = "";
        while (LoRa.available()) {
            received_message.data += (char)LoRa.read();
        }
        
        // Get signal quality
        received_message.rssi = LoRa.packetRssi();
        received_message.snr = LoRa.packetSnr();
        received_message.timestamp = millis();
        
        message_available = true;
        last_receive_time = millis();
        
        return true;
    }
    
    return false;
}

float LoRaRadio::calculateAirtime(size_t payload_bytes) const {
    // LoRa airtime calculation formula
    // Reference: Semtech SX1276 datasheet
    
    float sf = config.spreading_factor;
    float bw = config.bandwidth;
    float cr = config.coding_rate;
    
    // Symbol time in milliseconds
    float t_sym = (powf(2.0f, sf) / bw) * 1000.0f;
    
    // Preamble time (8 symbols + 4.25 symbols)
    float t_preamble = (8.0f + 4.25f) * t_sym;
    
    // Payload calculation
    float payload_symbols = 8.0f + max(ceilf((8.0f * payload_bytes - 4.0f * sf + 28.0f + 16.0f) / (4.0f * sf)) * (cr + 4.0f), 0.0f);
    
    float t_payload = payload_symbols * t_sym;
    
    // Total airtime in milliseconds
    return t_preamble + t_payload;
}
