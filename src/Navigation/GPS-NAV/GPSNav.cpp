#include "GPSNav.h"

#define RAD_TO_DEG 57.29578f
#define DEG_TO_RAD 0.0174533f
#define EARTH_RADIUS 6371000.0  // meters

GPSNav::GPSNav() : initialized(false), has_valid_heading(false),
                   nav_enabled(false), yaw_rate_command(0.0f),
                   last_update_time(0), last_gps_update_time(0) {
    x[0] = 0.0f;  // heading
    x[1] = 0.0f;  // heading_rate
    x[2] = 0.0f;  // heading_bias
    
    // Initialize covariance matrix with high uncertainty
    for(int i = 0; i < 3; i++) {
        for(int j = 0; j < 3; j++) {
            P[i][j] = (i == j) ? 100.0f : 0.0f;
        }
    }
    
    heading = 0.0f;
    heading_rate = 0.0f;
    heading_bias = 0.0f;
    target_lat = 0.0;
    target_lon = 0.0;
}

void GPSNav::update() {
    uint32_t now = millis();
    
    if(last_update_time == 0) {
        last_update_time = now;
        return;
    }
    
    float dt = (now - last_update_time) * 0.001f;  // seconds
    last_update_time = now;
    
    // Get IMU data
    float roll, pitch, yaw_unused;
    float rate_x, rate_y, rate_z;
    getAllIMUData(&roll, &pitch, &yaw_unused, &rate_x, &rate_y, &rate_z);
    
    // Convert rate_z from rad/s to deg/s
    float imu_rate_deg = rate_z * RAD_TO_DEG;
    
    // Prediction step (always run at high rate)
    predictKF(imu_rate_deg, dt);
    
    // Get GPS data
    double lat, lon;
    float alt, speed, course;
    uint8_t hour, minute, seconds, satellites;
    bool fix;
    getAllGPSDataV1(&lat, &lon, &alt, &speed, &hour, &minute, &seconds, 
                    &satellites, &fix, &course);
    
    // Update step (only when new GPS data available)
    static float last_course = -999.0f;
    if(fix && course != last_course) {
        last_course = course;
        last_gps_update_time = now;
        
        // Compute GPS heading quality based on speed
        float R = computeGPSHeadingQuality(speed);
        
        // Only use GPS heading if moving fast enough
        if(speed > MIN_GPS_HEADING_SPEED) {
            // Normalize GPS course to [-180, 180]
            float gps_heading = normalizeAngle(course);
            updateKF(gps_heading, R);
            has_valid_heading = true;
            initialized = true;
        } else if(speed < STATIONARY_SPEED) {
            // Stationary - increase process noise to allow slow drift
            // but don't update with GPS
            has_valid_heading = initialized;  // Keep last valid state
        }
    }
    
    // Extract state
    heading = x[0];
    heading_rate = x[1];
    heading_bias = x[2];
    
    // Navigation command computation
    if(nav_enabled && has_valid_heading && fix) {
        float track_desired = computeTrackToWaypoint(lat, lon);
        yaw_rate_command = computeNavigationCommand(track_desired);
    } else {
        yaw_rate_command = 0.0f;
    }
}

void GPSNav::predictKF(float imu_rate_deg, float dt) {
    // State transition
    // x[0] (heading) = x[0] + (imu_rate - bias) * dt
    // x[1] (rate) = imu_rate - bias
    // x[2] (bias) = x[2]
    
    float rate_corrected = imu_rate_deg - x[2];
    x[0] = normalizeAngle(x[0] + rate_corrected * dt);
    x[1] = rate_corrected;
    // x[2] unchanged (bias is slowly varying)
    
    // State transition matrix F
    float F[3][3] = {
        {1.0f, dt, -dt},
        {0.0f, 0.0f, -1.0f},
        {0.0f, 0.0f, 1.0f}
    };
    
    // Process noise covariance Q
    float Q[3][3] = {
        {PROCESS_NOISE_HEADING * dt, 0.0f, 0.0f},
        {0.0f, PROCESS_NOISE_RATE * dt, 0.0f},
        {0.0f, 0.0f, PROCESS_NOISE_BIAS * dt}
    };
    
    // P = F * P * F^T + Q
    float FP[3][3];
    matrixMultiply3x3(F, P, FP);
    
    float FPFT[3][3];
    for(int i = 0; i < 3; i++) {
        for(int j = 0; j < 3; j++) {
            FPFT[i][j] = 0.0f;
            for(int k = 0; k < 3; k++) {
                FPFT[i][j] += FP[i][k] * F[j][k];
            }
        }
    }
    
    matrixAdd3x3(FPFT, Q, P);
}

void GPSNav::updateKF(float gps_heading, float R) {
    // Measurement model: z = H * x where H = [1, 0, 0]
    // We only measure heading directly
    
    // Innovation
    float y = angleDifference(gps_heading, x[0]);
    
    // Innovation covariance S = H * P * H^T + R
    float S = P[0][0] + R;
    
    // Kalman gain K = P * H^T / S
    float K[3];
    K[0] = P[0][0] / S;
    K[1] = P[1][0] / S;
    K[2] = P[2][0] / S;
    
    // State update x = x + K * y
    x[0] = normalizeAngle(x[0] + K[0] * y);
    x[1] = x[1] + K[1] * y;
    x[2] = x[2] + K[2] * y;
    
    // Covariance update P = (I - K * H) * P
    float IKH[3][3] = {
        {1.0f - K[0], 0.0f, 0.0f},
        {-K[1], 1.0f, 0.0f},
        {-K[2], 0.0f, 1.0f}
    };
    
    float P_temp[3][3];
    matrixMultiply3x3(IKH, P, P_temp);
    
    for(int i = 0; i < 3; i++) {
        for(int j = 0; j < 3; j++) {
            P[i][j] = P_temp[i][j];
        }
    }
}

float GPSNav::normalizeAngle(float angle) {
    while(angle > 180.0f) angle -= 360.0f;
    while(angle < -180.0f) angle += 360.0f;
    return angle;
}

float GPSNav::angleDifference(float a, float b) {
    float diff = a - b;
    return normalizeAngle(diff);
}

float GPSNav::computeGPSHeadingQuality(float speed) {
    // Lower R (measurement noise) at higher speeds
    // R varies from MEASUREMENT_NOISE_GPS_HIGH at low speed
    // to MEASUREMENT_NOISE_GPS_LOW at high speed
    
    if(speed < MIN_GPS_HEADING_SPEED) {
        return MEASUREMENT_NOISE_GPS_HIGH;
    } else if(speed > 5.0f) {
        return MEASUREMENT_NOISE_GPS_LOW;
    } else {
        // Linear interpolation
        float t = (speed - MIN_GPS_HEADING_SPEED) / (5.0f - MIN_GPS_HEADING_SPEED);
        return MEASUREMENT_NOISE_GPS_HIGH + t * (MEASUREMENT_NOISE_GPS_LOW - MEASUREMENT_NOISE_GPS_HIGH);
    }
}

float GPSNav::computeTrackToWaypoint(double current_lat, double current_lon) {
    // Compute bearing from current position to waypoint
    // Using Haversine-based bearing formula
    
    double lat1 = current_lat * DEG_TO_RAD;
    double lon1 = current_lon * DEG_TO_RAD;
    double lat2 = target_lat * DEG_TO_RAD;
    double lon2 = target_lon * DEG_TO_RAD;
    
    double dLon = lon2 - lon1;
    
    double y = sin(dLon) * cos(lat2);
    double x = cos(lat1) * sin(lat2) - sin(lat1) * cos(lat2) * cos(dLon);
    
    float bearing = atan2(y, x) * RAD_TO_DEG;
    
    return normalizeAngle(bearing);
}

float GPSNav::computeNavigationCommand(float track_desired) {
    // Proportional controller for heading error
    float heading_error = angleDifference(track_desired, heading);
    
    // P-controller with gain
    float kp_nav = 0.5f;  // deg/s per deg error
    float command = kp_nav * heading_error;
    
    // Limit rate command
    float max_rate = 30.0f;  // deg/s
    if(command > max_rate) command = max_rate;
    if(command < -max_rate) command = -max_rate;
    
    return command;
}

void GPSNav::setWaypoint(double lat, double lon) {
    target_lat = lat;
    target_lon = lon;
}

void GPSNav::reset() {
    x[0] = 0.0f;
    x[1] = 0.0f;
    x[2] = 0.0f;
    
    for(int i = 0; i < 3; i++) {
        for(int j = 0; j < 3; j++) {
            P[i][j] = (i == j) ? 100.0f : 0.0f;
        }
    }
    
    initialized = false;
    has_valid_heading = false;
    yaw_rate_command = 0.0f;
}

void GPSNav::matrixMultiply3x3(float A[3][3], float B[3][3], float result[3][3]) {
    for(int i = 0; i < 3; i++) {
        for(int j = 0; j < 3; j++) {
            result[i][j] = 0.0f;
            for(int k = 0; k < 3; k++) {
                result[i][j] += A[i][k] * B[k][j];
            }
        }
    }
}

void GPSNav::matrixAdd3x3(float A[3][3], float B[3][3], float result[3][3]) {
    for(int i = 0; i < 3; i++) {
        for(int j = 0; j < 3; j++) {
            result[i][j] = A[i][j] + B[i][j];
        }
    }
}