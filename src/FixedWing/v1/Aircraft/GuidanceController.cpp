#include "GuidanceController.h"

GuidanceController::GuidanceController() 
    : num_waypoints(0), current_waypoint(0), mode(GUIDANCE_IDLE),
      waypoint_threshold_m(DEFAULT_WAYPOINT_THRESHOLD_M), mission_loitering(false) {
    loiter.lambda_c_rad = 0.0f;
    loiter.phi_c_rad = 0.0f;
    loiter.R_m = 500.0f;
    loiter.k_m = 125.0f;
    loiter.direction = 1;
}

void GuidanceController::loadMission(const Waypoint* wps, uint8_t count,
                                      float loiter_lambda_c_rad, float loiter_phi_c_rad,
                                      float loiter_R_m, float loiter_k_m, int loiter_dir) {
    if(count > MAX_WAYPOINTS) count = MAX_WAYPOINTS;
    
    for(uint8_t i = 0; i < count; i++) {
        waypoints[i] = wps[i];
    }
    
    num_waypoints = count;
    current_waypoint = 0;
    mission_loitering = false;
    
    loiter.lambda_c_rad = loiter_lambda_c_rad;
    loiter.phi_c_rad = loiter_phi_c_rad;
    loiter.R_m = loiter_R_m;
    loiter.k_m = loiter_k_m;
    loiter.direction = loiter_dir;
    
    if(count == 1) {
        mission_loitering = true;
        mode = GUIDANCE_MISSION;
    } else if(count > 1) {
        mode = GUIDANCE_MISSION;
    } else {
        mode = GUIDANCE_IDLE;
    }
}

void GuidanceController::activateStandaloneLoiter(float lambda_c_rad, float phi_c_rad,
                                                    float R_m, float k_m, int direction) {
    loiter.lambda_c_rad = lambda_c_rad;
    loiter.phi_c_rad = phi_c_rad;
    loiter.R_m = R_m;
    loiter.k_m = k_m;
    loiter.direction = direction;
    
    mode = GUIDANCE_STANDALONE_LOITER;
}

void GuidanceController::setWaypointThresholdMeters(float threshold_m) {
    waypoint_threshold_m = threshold_m;
}

float GuidanceController::update(float lambda_rad, float phi_rad) {
    if(mode == GUIDANCE_IDLE) {
        return 0.0f;
    }
    
    if(mode == GUIDANCE_MISSION) {
        if(mission_loitering) {
            return orbitGuidance(lambda_rad, phi_rad,
                               loiter.lambda_c_rad, loiter.phi_c_rad,
                               loiter.R_m, loiter.k_m, loiter.direction);
        }
        
        float lambda_target_rad = waypoints[current_waypoint].lambda_rad;
        float phi_target_rad = waypoints[current_waypoint].phi_rad;
        
        float distance_m = computeDistanceMeters(lambda_rad, phi_rad, 
                                                  lambda_target_rad, phi_target_rad);
        
        if(distance_m < waypoint_threshold_m) {
            advanceWaypoint();
            
            if(mission_loitering) {
                return orbitGuidance(lambda_rad, phi_rad,
                                   loiter.lambda_c_rad, loiter.phi_c_rad,
                                   loiter.R_m, loiter.k_m, loiter.direction);
            }
        }
        
        return optimalHeading(lambda_rad, phi_rad,
                            waypoints[current_waypoint].lambda_rad,
                            waypoints[current_waypoint].phi_rad);
    }
    
    if(mode == GUIDANCE_STANDALONE_LOITER) {
        return orbitGuidance(lambda_rad, phi_rad,
                           loiter.lambda_c_rad, loiter.phi_c_rad,
                           loiter.R_m, loiter.k_m, loiter.direction);
    }
    
    return 0.0f;
}

float GuidanceController::computeDistanceMeters(float lam1_rad, float phi1_rad,
                                                  float lam2_rad, float phi2_rad) const {
    float N_m, M_m;
    computeRadii(phi1_rad, &N_m, &M_m);
    
    float dx_m = N_m * cosf(phi1_rad) * (lam2_rad - lam1_rad);
    float dy_m = M_m * (phi2_rad - phi1_rad);
    
    return sqrtf(dx_m * dx_m + dy_m * dy_m);
}

float GuidanceController::getDistanceToTargetMeters(float lambda_rad, float phi_rad) const {
    if(mode == GUIDANCE_MISSION && !mission_loitering) {
        float lambda_target_rad = waypoints[current_waypoint].lambda_rad;
        float phi_target_rad = waypoints[current_waypoint].phi_rad;
        return computeDistanceMeters(lambda_rad, phi_rad, lambda_target_rad, phi_target_rad);
    }
    return 0.0f;
}

bool GuidanceController::isLoitering() const {
    if(mode == GUIDANCE_MISSION && mission_loitering) return true;
    if(mode == GUIDANCE_STANDALONE_LOITER) return true;
    return false;
}

void GuidanceController::advanceWaypoint() {
    current_waypoint++;
    
    if(current_waypoint >= num_waypoints) {
        mission_loitering = true;
        current_waypoint = num_waypoints - 1;
    }
}

void GuidanceController::reset() {
    num_waypoints = 0;
    current_waypoint = 0;
    mode = GUIDANCE_IDLE;
    mission_loitering = false;
}
