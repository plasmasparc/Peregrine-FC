#ifndef QMC5883_MANAGER_H
#define QMC5883_MANAGER_H

#include "QMC5883.h"
#include "pico/stdlib.h"
#include "hardware/timer.h"
#include "hardware/irq.h"

// ISR configuration - using ALARM 2 (IMU uses 0, GPS uses 1)
#define MAG_UPDATE_INTERVAL_US 100000  // 100ms -> 10Hz
#define MAG_ALARM_NUM 2
#define MAG_ALARM_IRQ TIMER_IRQ_2

// Thread-safe magnetometer data structure
struct MagManagerData {
    volatile int16_t x;
    volatile int16_t y;
    volatile int16_t z;
    volatile float heading;
};

// Public functions
void startMagnetometer();
void stopMagnetometer();
void getMagneticField(int16_t* x, int16_t* y, int16_t* z);
float getMagHeading();
void getAllMagData(int16_t* x, int16_t* y, int16_t* z, float* heading);

#endif
