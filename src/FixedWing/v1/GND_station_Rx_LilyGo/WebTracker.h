#ifndef WEB_TRACKER_H
#define WEB_TRACKER_H

#include <WiFi.h>
#include <WebServer.h>

#define MAX_POSITIONS 100
#define WIFI_SSID "attila"
#define WIFI_PASSWORD "attila12"

struct Position {
    double lat;
    double lon;
    uint32_t timestamp_ms;
};

class WebTracker {
public:
    WebTracker();
    void init();
    void addPosition(double lat, double lon);
    void handleClient();
    String getIPAddress();
    
private:
    WebServer server;
    Position positions[MAX_POSITIONS];
    uint16_t position_count;
    uint16_t position_index;
    
    void handleRoot();
    void handleData();
    String generateHTML();
};

#endif
