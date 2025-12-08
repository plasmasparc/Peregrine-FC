#include "IMU_Manager.h"

// Private volatile data - updated in ISR
static struct IMUData imu_data = {0};
static volatile bool isr_active = false;

// Forward declarations
static void scheduleAlarm(uint32_t delay_us);
static void alarmISR();

// ISR - runs every 1.5ms
static void alarmISR() {
    float temp_roll, temp_pitch, temp_yaw;
    float temp_rate_x, temp_rate_y, temp_rate_z;
    
    updateMPU6050();
    
    // Get data into temp variables
    getRollPitchYaw(&temp_roll, &temp_pitch, &temp_yaw);
    ::getBodyRates(&temp_rate_x, &temp_rate_y, &temp_rate_z);  // Call MPU6050's function
    
    // Atomic write to volatile data
    imu_data.roll = temp_roll;
    imu_data.pitch = temp_pitch;
    imu_data.yaw = temp_yaw;
    imu_data.rate_x = temp_rate_x;
    imu_data.rate_y = temp_rate_y;
    imu_data.rate_z = temp_rate_z;
    
    // Clear interrupt flag
    hw_clear_bits(&timer_hw->intr, 1u << ALARM_NUM);
    
    // Schedule next update
    if(isr_active) {
        scheduleAlarm(IMU_UPDATE_INTERVAL_US);
    }
}

static void scheduleAlarm(uint32_t delay_us) {
    hw_set_bits(&timer_hw->inte, 1u << ALARM_NUM);
    irq_set_exclusive_handler(ALARM_IRQ, alarmISR);
    irq_set_enabled(ALARM_IRQ, true);
    
    uint64_t target = timer_hw->timerawl + delay_us;
    timer_hw->alarm[ALARM_NUM] = (uint32_t)target;
}

// Public API
void startIMU() {
    initMPU6050();
    isr_active = true;
    scheduleAlarm(IMU_UPDATE_INTERVAL_US);
}

void stopIMU() {
    isr_active = false;
    irq_set_enabled(ALARM_IRQ, false);
    hw_clear_bits(&timer_hw->inte, 1u << ALARM_NUM);
}

void getAttitude(float* roll, float* pitch, float* yaw) {
    uint32_t status = save_and_disable_interrupts();
    *roll = imu_data.roll;
    *pitch = imu_data.pitch;
    *yaw = imu_data.yaw;
    restore_interrupts(status);
}

void getAllIMUData(float* roll, float* pitch, float* yaw,
                   float* rate_x, float* rate_y, float* rate_z) {
    uint32_t status = save_and_disable_interrupts();
    *roll = imu_data.roll;
    *pitch = imu_data.pitch;
    *yaw = imu_data.yaw;
    *rate_x = imu_data.rate_x;
    *rate_y = imu_data.rate_y;
    *rate_z = imu_data.rate_z;
    restore_interrupts(status);
}
