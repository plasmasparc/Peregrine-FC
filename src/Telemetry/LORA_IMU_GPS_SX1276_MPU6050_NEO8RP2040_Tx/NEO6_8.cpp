#include "NEO6_8.h"

static Adafruit_GPS GPS(&GPS_SERIAL);

void NEO6_8::init() {
    // Configure serial
    GPS_SERIAL.setRX(GPS_RX_PIN);
    GPS_SERIAL.setTX(GPS_TX_PIN);
    GPS_SERIAL.setFIFOSize(GPS_RX_BUFFER_SIZE);
    GPS_SERIAL.begin(GPS_BAUD_RATE);
    
    // Set update rate
    GPS.sendCommand(GPS_UPDATE_RATE);
    
    // Request RMC (recommended minimum) and GGA (fix data) only
    GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
    
    delay(100);
    
    gps_data = {0};
}

void NEO6_8::update() {
    while (GPS.available()) {
        char c = GPS.read();
        
        if (GPS.newNMEAreceived()) {
            if (GPS.parse(GPS.lastNMEA())) {
                if (GPS.fix) {
                    // Time
                    gps_data.year = GPS.year;
                    gps_data.month = GPS.month;
                    gps_data.day = GPS.day;
                    gps_data.hour = GPS.hour;
                    gps_data.minute = GPS.minute;
                    gps_data.seconds = GPS.seconds;
                    
                    // Position
                    gps_data.latitude = GPS.latitudeDegrees;
                    gps_data.longitude = GPS.longitudeDegrees;
                    gps_data.altitude = GPS.altitude;
                    
                    // Motion
                    gps_data.speed = GPS.speed * 0.51444f;  // knots to m/s
                    gps_data.course = GPS.angle;
                    
                    // Status
                    gps_data.fix = true;
                    gps_data.satellites = GPS.satellites;
                } else {
                    gps_data.fix = false;
                }
            }
        }
    }
}
