#ifndef NEO6_8_H
#define NEO6_8_H

#include <Adafruit_GPS.h>

// Pin configuration
#define GPS_RX_PIN 9
#define GPS_TX_PIN 8

// Serial configuration
#define GPS_SERIAL Serial2
#define GPS_BAUD_RATE 9600
#define GPS_RX_BUFFER_SIZE 4096  // ~200 NMEA sentences

// Update rate
#define GPS_UPDATE_RATE PMTK_SET_NMEA_UPDATE_2HZ  // 2Hz updates

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
    double latitude;   // degrees
    double longitude;  // degrees
    float altitude;    // meters
    
    // Motion
    float speed;       // m/s
    float course;      // degrees
    
    // Status
    bool fix;
    uint8_t satellites;
};

class NEO6_8 {
public:
    void init();
    void update();
    
    // Data access
    bool hasFix() const { return gps_data.fix; }
    const GPSData& getData() const { return gps_data; }
    
    // Individual getters
    double getLatitude() const { return gps_data.latitude; }
    double getLongitude() const { return gps_data.longitude; }
    float getAltitude() const { return gps_data.altitude; }
    float getSpeed() const { return gps_data.speed; }
    uint8_t getSatellites() const { return gps_data.satellites; }
    
private:
    GPSData gps_data;
};

#endif
