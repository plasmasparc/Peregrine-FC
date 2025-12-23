#include "NEO6_8.h"

void NEO6_8::init() {
    gps_serial = &Serial2;
    gps_serial->begin(GPS_BAUD_RATE, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
    
    gps_data.year = 0;
    gps_data.month = 0;
    gps_data.day = 0;
    gps_data.hour = 0;
    gps_data.minute = 0;
    gps_data.seconds = 0;
    gps_data.latitude = 0.0;
    gps_data.longitude = 0.0;
    gps_data.altitude = 0.0;
    gps_data.speed = 0.0;
    gps_data.course = 0.0;
    gps_data.fix = false;
    gps_data.satellites = 0;
}

void NEO6_8::update() {
    while(gps_serial->available() > 0) {
        gps.encode(gps_serial->read());
    }
    
    if(gps.location.isUpdated()) {
        gps_data.latitude = gps.location.lat();
        gps_data.longitude = gps.location.lng();
        gps_data.fix = gps.location.isValid();
    }
    
    if(gps.altitude.isUpdated()) {
        gps_data.altitude = gps.altitude.meters();
    }
    
    if(gps.speed.isUpdated()) {
        gps_data.speed = gps.speed.mps();
    }
    
    if(gps.course.isUpdated()) {
        gps_data.course = gps.course.deg();
    }
    
    if(gps.satellites.isUpdated()) {
        gps_data.satellites = gps.satellites.value();
    }
    
    if(gps.date.isUpdated()) {
        gps_data.year = gps.date.year();
        gps_data.month = gps.date.month();
        gps_data.day = gps.date.day();
    }
    
    if(gps.time.isUpdated()) {
        gps_data.hour = gps.time.hour();
        gps_data.minute = gps.time.minute();
        gps_data.seconds = gps.time.second();
    }
}
