#ifndef MPU6050_DUAL_H
#define MPU6050_DUAL_H

#include <Wire.h>
#include <math.h>

#define MPU6050_ADDR 0x68
#define REG_PWR_MGMT_1    0x6B
#define REG_GYRO_CONFIG   0x1B
#define REG_ACCEL_CONFIG  0x1C
#define REG_CONFIG        0x1A
#define REG_ACCEL_XOUT_H  0x3B

#define GYRO_RANGE_2000   0x18
#define ACCEL_RANGE_8G    0x10
#define DLPF_260HZ        0x00

#define ACCEL_X_OFFSET  233
#define ACCEL_Y_OFFSET  -91
#define ACCEL_Z_OFFSET  -316
#define GYRO_X_OFFSET   -43
#define GYRO_Y_OFFSET   1
#define GYRO_Z_OFFSET   -12

#define ACCEL_SCALE  4096.0f
#define GYRO_SCALE   16.384f
#define TWO_KP  1.0f
#define TWO_KI  0.0f
#define DEG_TO_RAD 0.017453293f

void initMPU6050Dual();
void getRollPitchYawSmoothed(float* roll, float* pitch, float* yaw);

#endif
