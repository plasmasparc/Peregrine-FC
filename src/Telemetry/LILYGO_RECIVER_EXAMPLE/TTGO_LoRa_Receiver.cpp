#include "TTGO_LoRa_Receiver.h"

void T3LoRaReceiver::initOLED() {
    Wire.begin(OLED_SDA, OLED_SCL);
    
    display = Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
    
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println("OLED failed");
        while(1);
    }
    
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("T3 LoRa Receiver");
    display.display();
}

void T3LoRaReceiver::initLoRa() {
    SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_CS);
    LoRa.setPins(LORA_CS, LORA_RST, LORA_DIO0);
    
    if (!LoRa.begin(LORA_FREQUENCY)) {
        Serial.println("LoRa failed");
        while(1);
    }
    
    LoRa.setSignalBandwidth(LORA_BANDWIDTH);
    LoRa.setSpreadingFactor(LORA_SPREADING_FACTOR);
    LoRa.setSyncWord(LORA_SYNC_WORD);
}

bool T3LoRaReceiver::init() {
    Serial.begin(115200);
    
    initOLED();
    initLoRa();
    
    data.valid = false;
    
    Serial.println("T3 receiver ready");
    return true;
}

bool T3LoRaReceiver::parseMessage(String msg) {
    data.valid = false;
    
    int idx;
    
    idx = msg.indexOf("R:");
    if (idx == -1) return false;
    data.roll = msg.substring(idx + 2, msg.indexOf(",", idx)).toFloat();
    
    idx = msg.indexOf("P:");
    if (idx == -1) return false;
    data.pitch = msg.substring(idx + 2, msg.indexOf(",", idx)).toFloat();
    
    idx = msg.indexOf("Y:");
    if (idx == -1) return false;
    data.yaw = msg.substring(idx + 2, msg.indexOf(",", idx)).toFloat();
    
    if (msg.indexOf("GPS:NOFIX") != -1) {
        data.gps_fix = false;
        data.valid = true;
        return true;
    }
    
    idx = msg.indexOf("LAT:");
    if (idx == -1) return false;
    data.latitude = msg.substring(idx + 4, msg.indexOf(",", idx)).toDouble();
    
    idx = msg.indexOf("LON:");
    if (idx == -1) return false;
    data.longitude = msg.substring(idx + 4, msg.indexOf(",", idx)).toDouble();
    
    idx = msg.indexOf("ALT:");
    if (idx == -1) return false;
    data.altitude = msg.substring(idx + 4, msg.indexOf(",", idx)).toFloat();
    
    idx = msg.indexOf("SPD:");
    if (idx == -1) return false;
    data.speed = msg.substring(idx + 4, msg.indexOf(",", idx)).toFloat();
    
    idx = msg.indexOf("SAT:");
    if (idx == -1) return false;
    data.satellites = msg.substring(idx + 4).toInt();
    
    data.gps_fix = true;
    data.valid = true;
    
    return true;
}

bool T3LoRaReceiver::receive() {
    int size = LoRa.parsePacket();
    
    if (size > 0) {
        String msg = "";
        while (LoRa.available()) {
            msg += (char)LoRa.read();
        }
        
        data.rssi = LoRa.packetRssi();
        data.snr = LoRa.packetSnr();
        data.timestamp = millis();
        
        Serial.println("RX: " + msg);
        Serial.print("RSSI: ");
        Serial.println(data.rssi);
        
        return parseMessage(msg);
    }
    
    return false;
}

void T3LoRaReceiver::displayOnOLED() {
    if (!data.valid) return;
    
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    
    display.print("R:");
    display.print(data.roll, 0);
    display.print(" P:");
    display.print(data.pitch, 0);
    display.print(" Y:");
    display.println(data.yaw, 0);
    
    if (data.gps_fix) {
        display.print("LAT:");
        display.println(data.latitude, 4);
        
        display.print("LON:");
        display.println(data.longitude, 4);
        
        display.print("ALT:");
        display.print(data.altitude, 0);
        display.print("m SPD:");
        display.println(data.speed, 0);
        
        display.print("SAT:");
        display.println(data.satellites);
    } else {
        display.println("GPS: NO FIX");
    }
    
    display.print("RSSI:");
    display.print(data.rssi);
    display.print(" SNR:");
    display.println(data.snr, 1);
    
    display.display();
}
