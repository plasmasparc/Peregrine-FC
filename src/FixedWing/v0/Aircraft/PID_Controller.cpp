#include "PID_Controller.h"

// Private volatile data
static volatile float target_roll = 0.0f;
static volatile float target_pitch = 0.0f;
static volatile float target_yaw = 0.0f;

static PIDGains gains_roll = {0.02f, 0.0f, 0.005f};
static PIDGains gains_pitch = {0.03f, 0.0f, 0.005f};
static PIDGains gains_yaw = {0.01f, 0.0f, 0.005f};

static PIDState state_roll = {0.0f, 0.0f};
static PIDState state_pitch = {0.0f, 0.0f};
static PIDState state_yaw = {0.0f, 0.0f};

static volatile bool pid_active = false;
static PWMController* pwm_ctrl = nullptr;

// Forward declarations
static void schedulePIDAlarm(uint32_t delay_us);
static void pidAlarmISR();
static float computePID(float error, PIDGains* gains, PIDState* state, float dt);
static float clamp(float value, float min, float max);

// PID computation
static float computePID(float error, PIDGains* gains, PIDState* state, float dt) {
    state->integral += error * dt;
    float derivative = (error - state->prev_error) / dt;
    state->prev_error = error;
    
    float output = gains->kp * error + gains->ki * state->integral + gains->kd * derivative;
    return output;
}

// Clamp function
static float clamp(float value, float min, float max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

// PID ISR - runs every 1.5ms on ALARM 2
static void pidAlarmISR() {
    float current_roll, current_pitch, current_yaw;
    float rate_x, rate_y, rate_z;
    
    // Get current attitude
    getAllIMUData(&current_roll, &current_pitch, &current_yaw,
                  &rate_x, &rate_y, &rate_z);
    
    // Calculate errors
    float error_roll = target_roll - current_roll;
    float error_pitch = target_pitch - current_pitch;
    float error_yaw = target_yaw - current_yaw;
    
    // Normalize yaw error to [-180, 180]
    while (error_yaw > 180.0f) error_yaw -= 360.0f;
    while (error_yaw < -180.0f) error_yaw += 360.0f;
    
    // Compute PID outputs
    float dt = PID_UPDATE_INTERVAL_US / 1000000.0f;
    float control_roll = computePID(error_roll, &gains_roll, &state_roll, dt);
    float control_pitch = computePID(error_pitch, &gains_pitch, &state_pitch, dt);
    float control_yaw = computePID(error_yaw, &gains_yaw, &state_yaw, dt);
    
    // Clamp outputs
    control_roll = clamp(control_roll, -MAX_CONTROL_OUTPUT, MAX_CONTROL_OUTPUT);
    control_pitch = clamp(control_pitch, -MAX_CONTROL_OUTPUT, MAX_CONTROL_OUTPUT);
    control_yaw = clamp(control_yaw, -MAX_CONTROL_OUTPUT, MAX_CONTROL_OUTPUT);
    
    // Apply to servos
    if (pwm_ctrl != nullptr) {
        pwm_ctrl->getChannel(0)->setNormalized(control_pitch);  // Elevator (pin 18)
        pwm_ctrl->getChannel(1)->setNormalized(control_yaw);    // Rudder (pin 19)
        pwm_ctrl->getChannel(2)->setNormalized(control_roll);   // Ailerons (pin 20)
    }
    
    // Clear interrupt flag
    hw_clear_bits(&timer_hw->intr, 1u << PID_ALARM_NUM);
    
    // Schedule next update
    if (pid_active) {
        schedulePIDAlarm(PID_UPDATE_INTERVAL_US);
    }
}

static void schedulePIDAlarm(uint32_t delay_us) {
    hw_set_bits(&timer_hw->inte, 1u << PID_ALARM_NUM);
    irq_set_exclusive_handler(PID_ALARM_IRQ, pidAlarmISR);
    irq_set_enabled(PID_ALARM_IRQ, true);
    
    uint64_t target = timer_hw->timerawl + delay_us;
    timer_hw->alarm[PID_ALARM_NUM] = (uint32_t)target;
}

// Public API
void startPID() {
    if (pwm_ctrl == nullptr) {
        pwm_ctrl = new PWMController(3);
        
        // Elevator configuration
        PWMConfig elevator = PWMPresets::customServo(18, 0, 2500);
        elevator.trim_us = 0;
        elevator.reversed = true;
        
        // Rudder configuration
        PWMConfig rudder = PWMPresets::customServo(19, 0, 2500);
        rudder.trim_us = 0;
        rudder.reversed = false;
        
        // Ailerons configuration
        PWMConfig ailerons = PWMPresets::customServo(20, 0, 2500);
        ailerons.trim_us = 0;
        ailerons.reversed = true;
        
        pwm_ctrl->addChannel(0, elevator);
        pwm_ctrl->addChannel(1, rudder);
        pwm_ctrl->addChannel(2, ailerons);
        pwm_ctrl->centerAll();
    }
    
    state_roll = {0.0f, 0.0f};
    state_pitch = {0.0f, 0.0f};
    state_yaw = {0.0f, 0.0f};
    
    pid_active = true;
    schedulePIDAlarm(PID_UPDATE_INTERVAL_US);
}

void stopPID() {
    pid_active = false;
    irq_set_enabled(PID_ALARM_IRQ, false);
    hw_clear_bits(&timer_hw->inte, 1u << PID_ALARM_NUM);
    
    if (pwm_ctrl != nullptr) {
        pwm_ctrl->centerAll();
    }
}

void setTargetRoll(float roll) {
    uint32_t status = save_and_disable_interrupts();
    target_roll = roll;
    restore_interrupts(status);
}

void setTargetPitch(float pitch) {
    uint32_t status = save_and_disable_interrupts();
    target_pitch = pitch;
    restore_interrupts(status);
}

void setTargetYaw(float yaw) {
    // Normalize to [-180, 180]
    while (yaw > 180.0f) yaw -= 360.0f;
    while (yaw < -180.0f) yaw += 360.0f;
    
    uint32_t status = save_and_disable_interrupts();
    target_yaw = yaw;
    restore_interrupts(status);
}

void setRollGains(float kp, float ki, float kd) {
    uint32_t status = save_and_disable_interrupts();
    gains_roll.kp = kp;
    gains_roll.ki = ki;
    gains_roll.kd = kd;
    restore_interrupts(status);
}

void setPitchGains(float kp, float ki, float kd) {
    uint32_t status = save_and_disable_interrupts();
    gains_pitch.kp = kp;
    gains_pitch.ki = ki;
    gains_pitch.kd = kd;
    restore_interrupts(status);
}

void setYawGains(float kp, float ki, float kd) {
    uint32_t status = save_and_disable_interrupts();
    gains_yaw.kp = kp;
    gains_yaw.ki = ki;
    gains_yaw.kd = kd;
    restore_interrupts(status);
}
