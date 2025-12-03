#include "DisplayManager.h"

DisplayManager::DisplayManager() : display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET) {
}

bool DisplayManager::init() {
    if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
        return false;
    }
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    return true;
}

void DisplayManager::showInitMessage(const char* message) {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println(message);
    display.display();
}

void DisplayManager::showDownlinkTelemetry(const DownlinkTelemetry* telem, uint32_t dl_count, uint32_t ul_count, uint32_t err_count) {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.print("DL:");
    display.print(dl_count);
    display.print(" UL:");
    display.print(ul_count);
    display.print(" E:");
    display.println(err_count);
    display.print("R:");
    display.print(telem->roll, 1);
    display.print(" P:");
    display.println(telem->pitch, 1);
    display.print("Y:");
    display.println(telem->yaw, 1);
    display.print("Alt:");
    display.print(telem->alt, 1);
    display.print(" Spd:");
    display.println(telem->speed, 1);
    display.print("Sats:");
    display.print(telem->satellites);
    display.print(" Fix:");
    display.println(telem->fix);
    display.display();
}

void DisplayManager::showUplinkControl(const UplinkControl* ctrl, uint32_t dl_count, uint32_t ul_count, uint32_t err_count) {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.print("DL:");
    display.print(dl_count);
    display.print(" UL:");
    display.print(ul_count);
    display.print(" E:");
    display.println(err_count);
    display.println("UPLINK CTRL");
    display.print("TgtR:");
    display.print(ctrl->target_roll, 1);
    display.print(" TgtP:");
    display.println(ctrl->target_pitch, 1);
    display.print("YawRate:");
    display.println(ctrl->yaw_rate, 2);
    display.print("Motor:");
    display.println(ctrl->motor_speed);
    display.display();
}

void DisplayManager::showNoSignal(uint32_t seconds_since_last) {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("NO SIGNAL");
    display.print("Last: ");
    display.print(seconds_since_last);
    display.println("s ago");
    display.display();
}
