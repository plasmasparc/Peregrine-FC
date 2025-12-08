#include "GPS_Manager.h"

// Private volatile data - updated in ISR
static struct GPSManagerData gps_manager_data = {0};
static volatile bool gps_isr_active = false;
static NEO6_8 gps;

// Forward declarations
static void scheduleGPSAlarm(uint32_t delay_us);
static void gpsAlarmISR();

// GPS ISR - runs every 500ms on ALARM 1
static void gpsAlarmISR() {
    // Update GPS (processes UART buffer)
    gps.update();
    
    if (gps.hasFix()) {
        // Get data from GPS
        const GPSData& data = gps.getData();
        
        // Atomic write to volatile data
        gps_manager_data.latitude = data.latitude;
        gps_manager_data.longitude = data.longitude;
        gps_manager_data.altitude = data.altitude;
        gps_manager_data.speed = data.speed;
        gps_manager_data.hour = data.hour;
        gps_manager_data.minute = data.minute;
        gps_manager_data.seconds = data.seconds;
        gps_manager_data.satellites = data.satellites;
        gps_manager_data.fix = true;
    } else {
        gps_manager_data.fix = false;
    }
    
    // Clear interrupt flag for ALARM 1
    hw_clear_bits(&timer_hw->intr, 1u << GPS_ALARM_NUM);
    
    // Schedule next update
    if (gps_isr_active) {
        scheduleGPSAlarm(GPS_UPDATE_INTERVAL_US);
    }
}

static void scheduleGPSAlarm(uint32_t delay_us) {
    hw_set_bits(&timer_hw->inte, 1u << GPS_ALARM_NUM);
    irq_set_exclusive_handler(GPS_ALARM_IRQ, gpsAlarmISR);
    irq_set_enabled(GPS_ALARM_IRQ, true);
    
    uint64_t target = timer_hw->timerawl + delay_us;
    timer_hw->alarm[GPS_ALARM_NUM] = (uint32_t)target;
}

// Public API
void startGPS() {
    gps.init();
    gps_isr_active = true;
    scheduleGPSAlarm(GPS_UPDATE_INTERVAL_US);
}

void stopGPS() {
    gps_isr_active = false;
    irq_set_enabled(GPS_ALARM_IRQ, false);
    hw_clear_bits(&timer_hw->inte, 1u << GPS_ALARM_NUM);
}

bool hasGPSFix() {
    uint32_t status = save_and_disable_interrupts();
    bool fix = gps_manager_data.fix;
    restore_interrupts(status);
    return fix;
}

void getGPSPosition(double* lat, double* lon, float* alt) {
    uint32_t status = save_and_disable_interrupts();
    *lat = gps_manager_data.latitude;
    *lon = gps_manager_data.longitude;
    *alt = gps_manager_data.altitude;
    restore_interrupts(status);
}

void getGPSTime(uint8_t* hour, uint8_t* minute, uint8_t* seconds) {
    uint32_t status = save_and_disable_interrupts();
    *hour = gps_manager_data.hour;
    *minute = gps_manager_data.minute;
    *seconds = gps_manager_data.seconds;
    restore_interrupts(status);
}

void getAllGPSData(double* lat, double* lon, float* alt, float* speed,
                   uint8_t* hour, uint8_t* minute, uint8_t* seconds,
                   uint8_t* satellites, bool* fix) {
    uint32_t status = save_and_disable_interrupts();
    *lat = gps_manager_data.latitude;
    *lon = gps_manager_data.longitude;
    *alt = gps_manager_data.altitude;
    *speed = gps_manager_data.speed;
    *hour = gps_manager_data.hour;
    *minute = gps_manager_data.minute;
    *seconds = gps_manager_data.seconds;
    *satellites = gps_manager_data.satellites;
    *fix = gps_manager_data.fix;
    restore_interrupts(status);
}
