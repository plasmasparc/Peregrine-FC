#ifndef PID_CONTROLLER_H
#define PID_CONTROLLER_H

#include "IMU_Manager.h"
#include "PWM_Controller.h"
#include "pico/stdlib.h"
#include "hardware/timer.h"
#include "hardware/irq.h"

// ISR configuration - using ALARM 2
#define PID_UPDATE_INTERVAL_US 5000  // 5ms -> ~200Hz (same as IMU)
#define PID_ALARM_NUM 2
#define PID_ALARM_IRQ TIMER_IRQ_2

// PID gains structure
struct PIDGains {
    float kp;
    float ki;
    float kd;
};

// PID state structure
struct PIDState {
    float integral;
    float prev_error;
};

// Control limits
#define MAX_CONTROL_OUTPUT 0.7f

// Public functions
void startPID();
void stopPID();
void setTargetRoll(float roll);
void setTargetPitch(float pitch);
void setTargetYaw(float yaw);
void setRollGains(float kp, float ki, float kd);
void setPitchGains(float kp, float ki, float kd);
void setYawGains(float kp, float ki, float kd);

#endif
