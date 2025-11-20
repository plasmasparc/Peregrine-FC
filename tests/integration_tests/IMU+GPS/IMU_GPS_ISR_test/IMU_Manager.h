#ifndef IMU_MANAGER_H
#define IMU_MANAGER_H

#include "MPU6050.h"
#include "pico/stdlib.h"
#include "hardware/timer.h"
#include "hardware/irq.h"

// ISR configuration
#define IMU_UPDATE_INTERVAL_US 1500  // 1.5ms -> ~666Hz
#define ALARM_NUM 0
#define ALARM_IRQ TIMER_IRQ_0

// Thread-safe data structure
struct IMUData {
    volatile float roll;
    volatile float pitch;
    volatile float yaw;
    volatile float rate_x;  // Body rate X (rad/s)
    volatile float rate_y;  // Body rate Y (rad/s)
    volatile float rate_z;  // Body rate Z (rad/s)
};

// Public functions
void startIMU();
void stopIMU();
void getAttitude(float* roll, float* pitch, float* yaw);
void getAllIMUData(float* roll, float* pitch, float* yaw, 
                   float* rate_x, float* rate_y, float* rate_z);

#endif
