#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define LORA_SS 18
#define LORA_RST 14
#define LORA_DIO0 26
#define LORA_MOSI 27
#define LORA_MISO 19
#define LORA_SCK 5

#define LORA_FREQUENCY 868E6
#define LORA_BANDWIDTH 125E3
#define LORA_SPREADING_FACTOR 7
#define LORA_SYNC_WORD 0xF3
#define LORA_TX_POWER 20
#define LORA_CODING_RATE 5

#define OLED_WIDTH 128
#define OLED_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(OLED_WIDTH, OLED_HEIGHT, &Wire, OLED_RESET);

#define PAYLOAD_SIZE 30
#define TIMEOUT_MS 130
#define CYCLE_DELAY_MS 91

uint8_t tx_buffer[PAYLOAD_SIZE];
uint8_t rx_buffer[PAYLOAD_SIZE];

uint32_t packets_sent = 0;
uint32_t packets_received = 0;
uint32_t packets_lost = 0;

void setup() {
    Serial.begin(115200);
    
    Wire.begin(21, 22);
    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println("OLED init failed");
        while(1);
    }
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("LoRa Test Init");
    display.display();
    
    SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_SS);
    LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);
    
    if(!LoRa.begin(LORA_FREQUENCY)) {
        display.println("LoRa init failed");
        display.display();
        while(1);
    }
    
    LoRa.setSignalBandwidth(LORA_BANDWIDTH);
    LoRa.setSpreadingFactor(LORA_SPREADING_FACTOR);
    LoRa.setSyncWord(LORA_SYNC_WORD);
    LoRa.setTxPower(LORA_TX_POWER, PA_OUTPUT_PA_BOOST_PIN);
    LoRa.setCodingRate4(LORA_CODING_RATE);
    //LoRa.setGain(0);
    //LoRa.enableCrc();  // Or disableCrc()
    //LoRa.disableCrc();
    
    display.println("LoRa OK");
    display.display();
    delay(1000);
}

void updateDisplay() {
    display.clearDisplay();
    display.setCursor(0, 0);
    
    display.print("Sent: ");
    display.println(packets_sent);
    
    display.print("Recv: ");
    display.println(packets_received);
    
    display.print("Lost: ");
    display.println(packets_lost);
    
    if(packets_sent > 0) {
        float success_rate = (float)packets_received / (float)packets_sent * 100.0f;
        display.print("Rate: ");
        display.print(success_rate, 1);
        display.println("%");
    }
    
    display.display();
}

void loop() {
    for(uint8_t i = 0; i < PAYLOAD_SIZE; i++) {
        tx_buffer[i] = random(0, 256);
    }
    
    LoRa.beginPacket();
    LoRa.write(tx_buffer, PAYLOAD_SIZE);
    LoRa.endPacket();
    packets_sent++;
    
    uint32_t start_time = millis();
    bool received = false;
    
    while(millis() - start_time < TIMEOUT_MS) {
        int packet_size = LoRa.parsePacket();
        
        if(packet_size == PAYLOAD_SIZE) {
            size_t idx = 0;
            while(LoRa.available() && idx < PAYLOAD_SIZE) {
                rx_buffer[idx++] = LoRa.read();
            }
            
            if(idx == PAYLOAD_SIZE) {
                bool match = true;
                for(uint8_t i = 0; i < PAYLOAD_SIZE; i++) {
                    if(rx_buffer[i] != tx_buffer[i]) {
                        match = false;
                        break;
                    }
                }
                
                if(match) {
                    packets_received++;
                    received = true;
                    break;
                }
            }
        }
    }
    
    if(!received) {
        packets_lost++;
    }
    
    updateDisplay();
    
    delay(CYCLE_DELAY_MS);
}
