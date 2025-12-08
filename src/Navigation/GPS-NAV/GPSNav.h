#ifndef GPS_NAV_H
#define GPS_NAV_H

#include "GPS_Manager.h"
#include "IMU_Manager.h"
#include <math.h>

// Update rates
#define GPS_NAV_UPDATE_RATE_MS 50  // 20Hz internal update

// Speed thresholds
#define MIN_GPS_HEADING_SPEED 1.5f  // m/s - minimum speed for valid GPS heading
#define STATIONARY_SPEED 0.3f        // m/s - consider stationary below this

// Kalman filter parameters
#define PROCESS_NOISE_HEADING 0.01f      // deg^2
#define PROCESS_NOISE_RATE 0.1f          // (deg/s)^2
#define PROCESS_NOISE_BIAS 0.0001f       // (deg/s)^2
#define MEASUREMENT_NOISE_GPS_HIGH 25.0f // deg^2 at low speed
#define MEASUREMENT_NOISE_GPS_LOW 4.0f   // deg^2 at high speed

class GPSNav {
public:
    GPSNav();
    
    void update();
    void reset();
    
    float getHeading() const { return heading; }
    float getHeadingRate() const { return heading_rate; }
    float getYawRateCommand() { return yaw_rate_command; }
    bool isValid() const { return initialized && has_valid_heading; }
    
    void setWaypoint(double lat, double lon);
    void enableNavigation(bool enable) { nav_enabled = enable; }
    
private:
    // Kalman filter state [heading, heading_rate, heading_bias]
    float x[3];
    float P[3][3];  // Covariance matrix
    
    // State variables
    float heading;           // deg (-180 to 180)
    float heading_rate;      // deg/s
    float heading_bias;      // deg/s
    
    // Navigation
    float yaw_rate_command;  // deg/s - output to flight controller
    double target_lat;
    double target_lon;
    bool nav_enabled;
    
    // Status
    bool initialized;
    bool has_valid_heading;
    uint32_t last_update_time;
    uint32_t last_gps_update_time;
    
    // Private methods
    void predictKF(float imu_rate_z, float dt);
    void updateKF(float gps_heading, float R);
    float normalizeAngle(float angle);
    float angleDifference(float a, float b);
    float computeGPSHeadingQuality(float speed);
    float computeTrackToWaypoint(double current_lat, double current_lon);
    float computeNavigationCommand(float track_desired);
    void matrixMultiply3x3(float A[3][3], float B[3][3], float result[3][3]);
    void matrixAdd3x3(float A[3][3], float B[3][3], float result[3][3]);
};

#endif