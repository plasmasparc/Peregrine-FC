#include "GuidanceController.h"

GuidanceController guidance;

void setup() {
    Serial.begin(115200);
    delay(2000);
    
    Serial.println("=== GuidanceController Demo ===\n");
    
    Waypoint mission[5];
    mission[0] = {0.41150f, 0.81550f};
    mission[1] = {0.41200f, 0.81600f};
    mission[2] = {0.41250f, 0.81650f};
    mission[3] = {0.41300f, 0.81600f};
    mission[4] = {0.41350f, 0.81550f};
    
    guidance.loadMission(mission, 5);
    guidance.setLoiterParams(0.41350f, 0.81550f, 500.0f, 125.0f, 1);
    guidance.setWaypointThreshold(0.00002f);
    
    Serial.print("Mission loaded: ");
    Serial.print(guidance.getTotalWaypoints());
    Serial.println(" waypoints\n");
    
    float lambda_rad = 0.41100f;
    float phi_rad = 0.81500f;
    
    for(int step = 0; step < 50; step++) {
        float psi_rad = guidance.update(lambda_rad, phi_rad);
        
        Serial.print("Step ");
        Serial.print(step);
        Serial.print(" | Mode: ");
        
        switch(guidance.getMode()) {
            case GUIDANCE_IDLE:   Serial.print("IDLE  "); break;
            case GUIDANCE_P2P:    Serial.print("P2P   "); break;
            case GUIDANCE_LOITER: Serial.print("LOITER"); break;
        }
        
        Serial.print(" | WP: ");
        Serial.print(guidance.getCurrentWaypoint());
        Serial.print("/");
        Serial.print(guidance.getTotalWaypoints());
        
        Serial.print(" | Dist: ");
        Serial.print(guidance.getDistanceToTarget(), 6);
        
        Serial.print(" | Psi: ");
        Serial.print(psi_rad * RAD_TO_DEG, 2);
        Serial.println(" deg");
        
        float N_m, M_m;
        computeRadii(phi_rad, &N_m, &M_m);
        float v_N_mps = 20.0f * cosf(psi_rad);
        float v_E_mps = 20.0f * sinf(psi_rad);
        float lambda_dot_radps = v_E_mps / (N_m * cosf(phi_rad));
        float phi_dot_radps = v_N_mps / M_m;
        
        lambda_rad += lambda_dot_radps * 0.5f;
        phi_rad += phi_dot_radps * 0.5f;
        
        if(guidance.getMode() == GUIDANCE_LOITER && step > 30) {
            break;
        }
    }
    
    Serial.println("\n=== Demo Complete ===");
}

void loop() {
}
