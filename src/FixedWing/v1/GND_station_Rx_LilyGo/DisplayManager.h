#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "FrameProtocol.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

class DisplayManager {
public:
    DisplayManager();
    bool init();
    void showInitMessage(const char* message);
    void showDownlinkTelemetry(const DownlinkTelemetry* telem, uint32_t dl_count, uint32_t ul_count, uint32_t err_count, const char* ip_addr);
    void showUplinkControl(const UplinkControl* ctrl, uint32_t dl_count, uint32_t ul_count, uint32_t err_count);
    void showNoSignal(uint32_t seconds_since_last);
    
private:
    Adafruit_SSD1306 display;
};

#endif
