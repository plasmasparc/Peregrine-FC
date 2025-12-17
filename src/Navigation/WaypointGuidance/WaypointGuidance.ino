#include "GuidanceController.h"

GuidanceController guidance;

void setup() {
    Serial.begin(115200);
    delay(2000);
    
    Serial.println("=== GuidanceController Demo ===\n");
    
    Serial.println("--- Test 1: Multi-waypoint Mission ---");
    testMultiWaypointMission();
    
    Serial.println("\n--- Test 2: Single Waypoint Mission (Auto-loiter) ---");
    testSingleWaypointMission();
    
    Serial.println("\n--- Test 3: Standalone Loiter ---");
    testStandaloneLoiter();
    
    Serial.println("\n=== All Tests Complete ===");
}

void testMultiWaypointMission() {
    guidance.reset();
    
    Waypoint mission[5];
    mission[0] = {0.41150f, 0.81550f};
    mission[1] = {0.41200f, 0.81600f};
    mission[2] = {0.41250f, 0.81650f};
    mission[3] = {0.41300f, 0.81600f};
    mission[4] = {0.41350f, 0.81550f};
    
    guidance.loadMission(mission, 5, 
                        0.41350f, 0.81550f, 500.0f, 125.0f, 1);
    guidance.setWaypointThresholdMeters(5.0f);
    
    Serial.print("Mission loaded: ");
    Serial.print(guidance.getTotalWaypoints());
    Serial.println(" waypoints");
    
    float lambda_rad = 0.41100f;
    float phi_rad = 0.81500f;
    
    for(int step = 0; step < 60; step++) {
        float psi_rad = guidance.update(lambda_rad, phi_rad);
        
        if(step % 10 == 0) {
            Serial.print("Step ");
            Serial.print(step);
            Serial.print(" | WP: ");
            Serial.print(guidance.getCurrentWaypoint());
            Serial.print("/");
            Serial.print(guidance.getTotalWaypoints());
            Serial.print(" | Loiter: ");
            Serial.print(guidance.isLoitering() ? "YES" : "NO ");
            Serial.print(" | Dist: ");
            Serial.print(guidance.getDistanceToTargetMeters(lambda_rad, phi_rad), 1);
            Serial.print(" m | Psi: ");
            Serial.print(psi_rad * RAD_TO_DEG, 1);
            Serial.println(" deg");
        }
        
        float N_m, M_m;
        computeRadii(phi_rad, &N_m, &M_m);
        float v_N_mps = 20.0f * cosf(psi_rad);
        float v_E_mps = 20.0f * sinf(psi_rad);
        float lambda_dot_radps = v_E_mps / (N_m * cosf(phi_rad));
        float phi_dot_radps = v_N_mps / M_m;
        
        lambda_rad += lambda_dot_radps * 0.5f;
        phi_rad += phi_dot_radps * 0.5f;
        
        if(guidance.isLoitering() && step > 40) break;
    }
}

void testSingleWaypointMission() {
    guidance.reset();
    
    Waypoint mission[1];
    mission[0] = {0.41200f, 0.81600f};
    
    guidance.loadMission(mission, 1,
                        0.41200f, 0.81600f, 300.0f, 75.0f, -1);
    guidance.setWaypointThresholdMeters(6.0f);
    
    Serial.println("Single waypoint mission (should immediately loiter)");
    
    float lambda_rad = 0.41180f;
    float phi_rad = 0.81580f;
    
    for(int step = 0; step < 30; step++) {
        float psi_rad = guidance.update(lambda_rad, phi_rad);
        
        if(step % 10 == 0) {
            Serial.print("Step ");
            Serial.print(step);
            Serial.print(" | Loiter: ");
            Serial.print(guidance.isLoitering() ? "YES" : "NO ");
            Serial.print(" | Psi: ");
            Serial.print(psi_rad * RAD_TO_DEG, 1);
            Serial.println(" deg");
        }
        
        float N_m, M_m;
        computeRadii(phi_rad, &N_m, &M_m);
        float v_N_mps = 15.0f * cosf(psi_rad);
        float v_E_mps = 15.0f * sinf(psi_rad);
        float lambda_dot_radps = v_E_mps / (N_m * cosf(phi_rad));
        float phi_dot_radps = v_N_mps / M_m;
        
        lambda_rad += lambda_dot_radps * 0.5f;
        phi_rad += phi_dot_radps * 0.5f;
    }
}

void testStandaloneLoiter() {
    guidance.reset();
    
    guidance.activateStandaloneLoiter(0.41000f, 0.81500f, 400.0f, 100.0f, 1);
    
    Serial.println("Standalone loiter mode activated");
    
    float lambda_rad = 0.40950f;
    float phi_rad = 0.81450f;
    
    for(int step = 0; step < 30; step++) {
        float psi_rad = guidance.update(lambda_rad, phi_rad);
        
        if(step % 10 == 0) {
            Serial.print("Step ");
            Serial.print(step);
            Serial.print(" | Mode: STANDALONE_LOITER");
            Serial.print(" | Psi: ");
            Serial.print(psi_rad * RAD_TO_DEG, 1);
            Serial.println(" deg");
        }
        
        float N_m, M_m;
        computeRadii(phi_rad, &N_m, &M_m);
        float v_N_mps = 25.0f * cosf(psi_rad);
        float v_E_mps = 25.0f * sinf(psi_rad);
        float lambda_dot_radps = v_E_mps / (N_m * cosf(phi_rad));
        float phi_dot_radps = v_N_mps / M_m;
        
        lambda_rad += lambda_dot_radps * 0.5f;
        phi_rad += phi_dot_radps * 0.5f;
    }
}

void loop() {
}
