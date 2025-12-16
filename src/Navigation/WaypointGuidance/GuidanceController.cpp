#include "GuidanceController.h"

GuidanceController::GuidanceController() 
    : num_waypoints(0), current_waypoint(0), mode(GUIDANCE_IDLE),
      waypoint_threshold_rad(DEFAULT_WAYPOINT_THRESHOLD_RAD), last_distance_rad(0.0f) {
    loiter.lambda_c_rad = 0.0f;
    loiter.phi_c_rad = 0.0f;
    loiter.R_m = 500.0f;
    loiter.k_m = 125.0f;
    loiter.direction = 1;
}

void GuidanceController::loadMission(const Waypoint* wps, uint8_t count) {
    if(count > MAX_WAYPOINTS) count = MAX_WAYPOINTS;
    
    for(uint8_t i = 0; i < count; i++) {
        waypoints[i] = wps[i];
    }
    
    num_waypoints = count;
    current_waypoint = 0;
    mode = (count > 0) ? GUIDANCE_P2P : GUIDANCE_IDLE;
}

void GuidanceController::setLoiterParams(float lam_c_rad, float phi_c_rad, float R_m, float k_m, int dir) {
    loiter.lambda_c_rad = lam_c_rad;
    loiter.phi_c_rad = phi_c_rad;
    loiter.R_m = R_m;
    loiter.k_m = k_m;
    loiter.direction = dir;
}

void GuidanceController::setWaypointThreshold(float threshold_rad) {
    waypoint_threshold_rad = threshold_rad;
}

float GuidanceController::update(float lambda_rad, float phi_rad) {
    if(mode == GUIDANCE_IDLE) {
        return 0.0f;
    }
    
    if(mode == GUIDANCE_P2P) {
        float lambda_target_rad = waypoints[current_waypoint].lambda_rad;
        float phi_target_rad = waypoints[current_waypoint].phi_rad;
        
        last_distance_rad = computeDistance(lambda_rad, phi_rad, lambda_target_rad, phi_target_rad);
        
        if(last_distance_rad < waypoint_threshold_rad) {
            advanceWaypoint();
        }
        
        if(mode == GUIDANCE_P2P) {
            return optimalHeading(lambda_rad, phi_rad, 
                                waypoints[current_waypoint].lambda_rad,
                                waypoints[current_waypoint].phi_rad);
        }
    }
    
    if(mode == GUIDANCE_LOITER) {
        return orbitGuidance(lambda_rad, phi_rad, 
                           loiter.lambda_c_rad, loiter.phi_c_rad,
                           loiter.R_m, loiter.k_m, loiter.direction);
    }
    
    return 0.0f;
}

float GuidanceController::computeDistance(float lam1_rad, float phi1_rad, float lam2_rad, float phi2_rad) {
    float dlam_rad = lam2_rad - lam1_rad;
    float dphi_rad = phi2_rad - phi1_rad;
    return sqrtf(dlam_rad * dlam_rad + dphi_rad * dphi_rad);
}

void GuidanceController::advanceWaypoint() {
    current_waypoint++;
    
    if(current_waypoint >= num_waypoints) {
        mode = GUIDANCE_LOITER;
        current_waypoint = num_waypoints - 1;
    }
}

void GuidanceController::reset() {
    num_waypoints = 0;
    current_waypoint = 0;
    mode = GUIDANCE_IDLE;
    last_distance_rad = 0.0f;
}
