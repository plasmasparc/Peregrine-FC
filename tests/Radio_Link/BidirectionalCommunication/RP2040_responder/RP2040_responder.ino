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

#define PAYLOAD_SIZE 30

uint8_t rx_buffer[PAYLOAD_SIZE];

void setup() {
    Serial.begin(115200);
    delay(2000);
    
    Serial.println("Init SPI...");
    SPI.setRX(LORA_RX_PIN);
    SPI.setTX(LORA_TX_PIN);
    SPI.setSCK(LORA_SCK_PIN);
    SPI.begin();
    
    Serial.println("Init LoRa...");
    LoRa.setPins(LORA_SS_PIN, LORA_RST_PIN, LORA_DIO0_PIN);
    
    if(!LoRa.begin(LORA_FREQUENCY)) {
        Serial.println("LoRa init FAILED");
        while(1);
    }
    
    Serial.println("Configure LoRa...");
    LoRa.setSignalBandwidth(LORA_BANDWIDTH);
    LoRa.setSpreadingFactor(LORA_SPREADING_FACTOR);
    LoRa.setSyncWord(LORA_SYNC_WORD);
    LoRa.setTxPower(LORA_TX_POWER, PA_OUTPUT_PA_BOOST_PIN);
    LoRa.setCodingRate4(LORA_CODING_RATE);
    
    Serial.println("RP2040 Responder Ready");
}

void loop() {
    int packet_size = LoRa.parsePacket();
    
    if(packet_size > 0) {
        Serial.print("RX size: ");
        Serial.println(packet_size);
        
        size_t idx = 0;
        while(LoRa.available() && idx < PAYLOAD_SIZE) {
            rx_buffer[idx++] = LoRa.read();
        }
        
        int rssi = LoRa.packetRssi();
        float snr = LoRa.packetSnr();
        
        Serial.print("Read ");
        Serial.print(idx);
        Serial.print(" bytes | RSSI: ");
        Serial.print(rssi);
        Serial.print(" | SNR: ");
        Serial.println(snr);
        
        if(idx == PAYLOAD_SIZE) {
            LoRa.beginPacket();
            LoRa.write(rx_buffer, PAYLOAD_SIZE);
            LoRa.endPacket();
            
            Serial.println("Echo sent");
        }
    }
    delay(100);
}
