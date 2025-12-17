#include <SPI.h>
#include <LoRa.h>

#define LORA_RX_PIN 0
#define LORA_TX_PIN 3
#define LORA_SCK_PIN 2
#define LORA_SS_PIN 1
#define LORA_RST_PIN 6
#define LORA_DIO0_PIN 5

#define LORA_FREQUENCY 868E6
#define LORA_BANDWIDTH 125E3
#define LORA_SPREADING_FACTOR 7
#define LORA_SYNC_WORD 0xF3
#define LORA_TX_POWER 20
#define LORA_CODING_RATE 5

#define PAYLOAD_SIZE 30

uint8_t rx_buffer[PAYLOAD_SIZE];
uint32_t last_debug = 0;

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
    
    LoRa.receive();
    
    Serial.println("RP2040 Responder Ready");
    Serial.print("Frequency: ");
    Serial.println(LORA_FREQUENCY);
    Serial.print("Bandwidth: ");
    Serial.println(LORA_BANDWIDTH);
    Serial.print("SF: ");
    Serial.println(LORA_SPREADING_FACTOR);
    Serial.print("Sync: 0x");
    Serial.println(LORA_SYNC_WORD, HEX);
}

void loop() {
    int packet_size = LoRa.parsePacket();
    
    if(packet_size != 0) {
        Serial.print("Packet size: ");
        Serial.println(packet_size);
        
        if(packet_size == PAYLOAD_SIZE) {
            size_t idx = 0;
            while(LoRa.available() && idx < PAYLOAD_SIZE) {
                rx_buffer[idx++] = LoRa.read();
            }
            
            if(idx == PAYLOAD_SIZE) {
                int rssi = LoRa.packetRssi();
                float snr = LoRa.packetSnr();
                
                Serial.print("RX OK | RSSI: ");
                Serial.print(rssi);
                Serial.print(" | SNR: ");
                Serial.println(snr);
                
                delay(5);
                
                LoRa.beginPacket();
                LoRa.write(rx_buffer, PAYLOAD_SIZE);
                LoRa.endPacket();
                
                Serial.println("Echo sent");
                
                LoRa.receive();
            } else {
                Serial.println("Partial read");
            }
        }
    }
    
    if(millis() - last_debug > 5000) {
        Serial.println("Listening...");
        last_debug = millis();
    }
}
