#include "QMC5883_Manager.h"

// Private volatile data - updated in ISR
static struct MagManagerData mag_manager_data = {0};
static volatile bool mag_isr_active = false;
static QMC5883 compass;

// Forward declarations
static void scheduleMagAlarm(uint32_t delay_us);
static void magAlarmISR();

// Magnetometer ISR - runs every 100ms on ALARM 2
static void magAlarmISR() {
    // Update magnetometer data
    compass.update();
    
    // Get data
    const MagData& data = compass.getData();
    
    // Atomic write to volatile data
    mag_manager_data.x = data.x;
    mag_manager_data.y = data.y;
    mag_manager_data.z = data.z;
    mag_manager_data.heading = data.heading;
    
    // Clear interrupt flag for ALARM 2
    hw_clear_bits(&timer_hw->intr, 1u << MAG_ALARM_NUM);
    
    // Schedule next update
    if (mag_isr_active) {
        scheduleMagAlarm(MAG_UPDATE_INTERVAL_US);
    }
}

static void scheduleMagAlarm(uint32_t delay_us) {
    hw_set_bits(&timer_hw->inte, 1u << MAG_ALARM_NUM);
    irq_set_exclusive_handler(MAG_ALARM_IRQ, magAlarmISR);
    irq_set_enabled(MAG_ALARM_IRQ, true);
    
    uint64_t target = timer_hw->timerawl + delay_us;
    timer_hw->alarm[MAG_ALARM_NUM] = (uint32_t)target;
}

// Public API
void startMagnetometer() {
    compass.init();
    mag_isr_active = true;
    scheduleMagAlarm(MAG_UPDATE_INTERVAL_US);
}

void stopMagnetometer() {
    mag_isr_active = false;
    irq_set_enabled(MAG_ALARM_IRQ, false);
    hw_clear_bits(&timer_hw->inte, 1u << MAG_ALARM_NUM);
}

void getMagneticField(int16_t* x, int16_t* y, int16_t* z) {
    uint32_t status = save_and_disable_interrupts();
    *x = mag_manager_data.x;
    *y = mag_manager_data.y;
    *z = mag_manager_data.z;
    restore_interrupts(status);
}

float getMagHeading() {
    uint32_t status = save_and_disable_interrupts();
    float heading = mag_manager_data.heading;
    restore_interrupts(status);
    return heading;
}

void getAllMagData(int16_t* x, int16_t* y, int16_t* z, float* heading) {
    uint32_t status = save_and_disable_interrupts();
    *x = mag_manager_data.x;
    *y = mag_manager_data.y;
    *z = mag_manager_data.z;
    *heading = mag_manager_data.heading;
    restore_interrupts(status);
}
