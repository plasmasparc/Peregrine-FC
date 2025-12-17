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

uint32_t packet_count = 0;

void setup() {
    Serial.begin(115200);
    delay(2000);
    
    Serial.println("RP2040 Hardware Test");
    
    SPI.setRX(LORA_RX_PIN);
    SPI.setTX(LORA_TX_PIN);
    SPI.setSCK(LORA_SCK_PIN);
    SPI.begin();
    
    LoRa.setPins(LORA_SS_PIN, LORA_RST_PIN, LORA_DIO0_PIN);
    
    if(!LoRa.begin(LORA_FREQUENCY)) {
        Serial.println("LoRa FAILED");
        while(1);
    }
    
    LoRa.setSignalBandwidth(LORA_BANDWIDTH);
    LoRa.setSpreadingFactor(LORA_SPREADING_FACTOR);
    LoRa.setSyncWord(LORA_SYNC_WORD);
    LoRa.setTxPower(LORA_TX_POWER, PA_OUTPUT_PA_BOOST_PIN);
    LoRa.setCodingRate4(LORA_CODING_RATE);
    
    Serial.println("TX Ready");
}

void loop() {
    LoRa.beginPacket();
    LoRa.print("PKT:");
    LoRa.print(packet_count);
    LoRa.endPacket();
    
    Serial.print("TX ");
    Serial.println(packet_count);
    
    packet_count++;
    delay(100);
}
