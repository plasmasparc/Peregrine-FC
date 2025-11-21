#ifndef MPU6050_H
#define MPU6050_H

#include <Wire.h>
#include <math.h>

// I2C Address
#define MPU6050_ADDR 0x68

// Register addresses
#define REG_PWR_MGMT_1    0x6B
#define REG_GYRO_CONFIG   0x1B
#define REG_ACCEL_CONFIG  0x1C
#define REG_CONFIG        0x1A
#define REG_ACCEL_XOUT_H  0x3B
#define REG_SELF_TEST_X   0x0D
#define REG_SELF_TEST_Y   0x0E
#define REG_SELF_TEST_Z   0x0F
#define REG_SELF_TEST_A   0x10

// Sensor configuration
#define GYRO_RANGE_2000   0x18  // ±2000°/s
#define ACCEL_RANGE_8G    0x10  // ±8g
#define DLPF_260HZ        0x00  // 260Hz bandwidth

// Calibration offsets
#define ACCEL_X_OFFSET  327
#define ACCEL_Y_OFFSET  54
#define ACCEL_Z_OFFSET  -590
#define GYRO_X_OFFSET   -7
#define GYRO_Y_OFFSET   15
#define GYRO_Z_OFFSET   -62


// Scale factors
#define ACCEL_SCALE  4096.0f   // LSB/g for ±8g
#define GYRO_SCALE   16.384f   // LSB/(°/s) for ±2000°/s

// Mahony filter gains
#define TWO_KP  1.0f  // Proportional gain
#define TWO_KI  0.0f  // Integral gain

// Function declarations
void initMPU6050();
void updateMPU6050();
void getRollPitchYaw(float* roll, float* pitch, float* yaw);
void getBodyRates(float* rate_x, float* rate_y, float* rate_z);
void getQuaternion(float* q0, float* q1, float* q2, float* q3);

#endif
