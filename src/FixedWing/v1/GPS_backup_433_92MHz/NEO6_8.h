#ifndef NEO6_8_H
#define NEO6_8_H

#include <TinyGPS++.h>
#include <HardwareSerial.h>

// Pin configuration - LilyGO T-Display available pins
#define GPS_RX_PIN 34
#define GPS_TX_PIN 14

// Serial configuration
#define GPS_BAUD_RATE 9600

// Update rate
#define GPS_UPDATE_RATE_MS 500

// GPS data structure
struct GPSData {
    // Time
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t seconds;
    
    // Position
    double latitude;
    double longitude;
    float altitude;
    
    // Motion
    float speed;
    float course;
    
    // Status
    bool fix;
    uint8_t satellites;
};

class NEO6_8 {
public:
    void init();
    void update();
    
    bool hasFix() const { return gps_data.fix; }
    const GPSData& getData() const { return gps_data; }
    
    double getLatitude() const { return gps_data.latitude; }
    double getLongitude() const { return gps_data.longitude; }
    float getAltitude() const { return gps_data.altitude; }
    float getSpeed() const { return gps_data.speed; }
    uint8_t getSatellites() const { return gps_data.satellites; }
    
private:
    GPSData gps_data;
    TinyGPSPlus gps;
    HardwareSerial* gps_serial;
};

#endif
