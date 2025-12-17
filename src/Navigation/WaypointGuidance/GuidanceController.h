#ifndef GUIDANCE_CONTROLLER_H
#define GUIDANCE_CONTROLLER_H

#include "GuidanceMath.h"
#include <stdint.h>

#define MAX_WAYPOINTS 100
#define DEFAULT_WAYPOINT_THRESHOLD_M 5.0f

enum GuidanceMode {
    GUIDANCE_IDLE,
    GUIDANCE_MISSION,
    GUIDANCE_STANDALONE_LOITER
};

struct Waypoint {
    float lambda_rad;
    float phi_rad;
};

struct LoiterParams {
    float lambda_c_rad;
    float phi_c_rad;
    float R_m;
    float k_m;
    int direction;
};

class GuidanceController {
public:
    GuidanceController();
    
    void loadMission(const Waypoint* wps, uint8_t count, 
                     float loiter_lambda_c_rad, float loiter_phi_c_rad,
                     float loiter_R_m, float loiter_k_m, int loiter_dir);
    
    void activateStandaloneLoiter(float lambda_c_rad, float phi_c_rad,
                                   float R_m, float k_m, int direction);
    
    void setWaypointThresholdMeters(float threshold_m);
    
    float update(float lambda_rad, float phi_rad);
    
    GuidanceMode getMode() const { return mode; }
    uint8_t getCurrentWaypoint() const { return current_waypoint; }
    uint8_t getTotalWaypoints() const { return num_waypoints; }
    bool isLoitering() const;
    float getDistanceToTargetMeters(float lambda_rad, float phi_rad) const;
    
    void reset();
    
private:
    Waypoint waypoints[MAX_WAYPOINTS];
    uint8_t num_waypoints;
    uint8_t current_waypoint;
    GuidanceMode mode;
    LoiterParams loiter;
    float waypoint_threshold_m;
    bool mission_loitering;
    
    float computeDistanceMeters(float lam1_rad, float phi1_rad, 
                                 float lam2_rad, float phi2_rad) const;
    void advanceWaypoint();
};

#endif
