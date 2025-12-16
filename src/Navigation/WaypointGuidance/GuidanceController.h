#ifndef GUIDANCE_CONTROLLER_H
#define GUIDANCE_CONTROLLER_H

#include "GuidanceMath.h"
#include <stdint.h>

#define MAX_WAYPOINTS 100
#define DEFAULT_WAYPOINT_THRESHOLD_RAD 0.00002f

enum GuidanceMode {
    GUIDANCE_IDLE,
    GUIDANCE_P2P,
    GUIDANCE_LOITER
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
    
    void loadMission(const Waypoint* wps, uint8_t count);
    void setLoiterParams(float lam_c_rad, float phi_c_rad, float R_m, float k_m, int dir);
    void setWaypointThreshold(float threshold_rad);
    
    float update(float lambda_rad, float phi_rad);
    
    GuidanceMode getMode() const { return mode; }
    uint8_t getCurrentWaypoint() const { return current_waypoint; }
    uint8_t getTotalWaypoints() const { return num_waypoints; }
    bool isComplete() const { return mode == GUIDANCE_LOITER; }
    float getDistanceToTarget() const { return last_distance_rad; }
    
    void reset();
    
private:
    Waypoint waypoints[MAX_WAYPOINTS];
    uint8_t num_waypoints;
    uint8_t current_waypoint;
    GuidanceMode mode;
    LoiterParams loiter;
    float waypoint_threshold_rad;
    float last_distance_rad;
    
    float computeDistance(float lam1_rad, float phi1_rad, float lam2_rad, float phi2_rad);
    void advanceWaypoint();
};

#endif
