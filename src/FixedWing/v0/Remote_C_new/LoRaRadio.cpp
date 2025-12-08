#include "LoRaRadio.h"

LoRaRadio::LoRaRadio() {
    config.ss_pin = LORA_SS_PIN;
    config.rst_pin = LORA_RST_PIN;
    config.dio0_pin = LORA_DIO0_PIN;
    config.sck_pin = LORA_SCK_PIN;
    config.miso_pin = LORA_MISO_PIN;
    config.mosi_pin = LORA_MOSI_PIN;
    config.frequency = LORA_FREQUENCY;
    config.bandwidth = LORA_BANDWIDTH;
    config.spreading_factor = LORA_SPREADING_FACTOR;
    config.sync_word = LORA_SYNC_WORD;
    config.tx_power = LORA_TX_POWER;
    config.coding_rate = LORA_CODING_RATE;
}

bool LoRaRadio::init() {
    SPI.begin(config.sck_pin, config.miso_pin, config.mosi_pin, config.ss_pin);
    LoRa.setPins(config.ss_pin, config.rst_pin, config.dio0_pin);
    
    if (!LoRa.begin(config.frequency)) {
        return false;
    }
    
    LoRa.setSignalBandwidth(config.bandwidth);
    LoRa.setSpreadingFactor(config.spreading_factor);
    LoRa.setSyncWord(config.sync_word);
    LoRa.setTxPower(config.tx_power);
    LoRa.setCodingRate4(config.coding_rate);
    
    return true;
}

void LoRaRadio::sendFrame(const uint8_t* frame, size_t size) {
    LoRa.beginPacket();
    LoRa.write(frame, size);
    LoRa.endPacket();
}

size_t LoRaRadio::receiveFrame(uint8_t* buffer, size_t max_size) {
    int packet_size = LoRa.parsePacket();
    
    if (packet_size <= 0) {
        return 0;
    }
    
    size_t bytes_to_read = min((size_t)packet_size, max_size);
    size_t bytes_read = 0;

    while (LoRa.available() && bytes_read < bytes_to_read) {
        buffer[bytes_read++] = (uint8_t)LoRa.read();
    }
    
    return bytes_read;
}
