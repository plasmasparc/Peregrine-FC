#ifndef GPS_MANAGER_H
#define GPS_MANAGER_H

#include "NEO6_8.h"
#include "pico/stdlib.h"
#include "hardware/timer.h"
#include "hardware/irq.h"

// ISR configuration - using ALARM 1 (IMU uses ALARM 0)
#define GPS_UPDATE_INTERVAL_US 500000  // 500ms -> 2Hz
#define GPS_ALARM_NUM 1
#define GPS_ALARM_IRQ TIMER_IRQ_1

// Thread-safe GPS data structure
struct GPSManagerData {
    volatile double latitude;
    volatile double longitude;
    volatile float altitude;
    volatile float speed;
    volatile uint8_t hour;
    volatile uint8_t minute;
    volatile uint8_t seconds;
    volatile uint8_t satellites;
    volatile bool fix;
};

// Public functions
void startGPS();
void stopGPS();
bool hasGPSFix();
void getGPSPosition(double* lat, double* lon, float* alt);
void getGPSTime(uint8_t* hour, uint8_t* minute, uint8_t* seconds);
void getAllGPSData(double* lat, double* lon, float* alt, float* speed, 
                   uint8_t* hour, uint8_t* minute, uint8_t* seconds,
                   uint8_t* satellites, bool* fix);

#endif
