#ifndef WEB_TRACKER_H
#define WEB_TRACKER_H

#include <WiFi.h>
#include <WebServer.h>

#define MAX_POSITIONS 100

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
    
private:
    WebServer server;
    Position positions[MAX_POSITIONS];
    uint16_t position_count;
    uint16_t position_index;
    
    void handleRoot();
    String generateHTML();
};

#endif
