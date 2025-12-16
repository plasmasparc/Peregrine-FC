#include "GuidanceMath.h"

#define MODE_P2P 0
#define MODE_LOITER 1

void setup() {
    Serial.begin(115200);
    while(!Serial);
    delay(100);
    Serial.println("READY");
}

void loop() {
    if(Serial.available() > 0) {
        String line = Serial.readStringUntil('\n');
        
        int parts[20];
        int part_count = 0;
        int start_idx = 0;
        
        for(int i = 0; i <= line.length(); i++) {
            if(i == line.length() || line[i] == ',') {
                parts[part_count++] = start_idx;
                start_idx = i + 1;
                if(part_count >= 20) break;
            }
        }
        
        if(part_count < 3) return;
        
        int mode = line.substring(parts[0], parts[1] - 1).toInt();
        float lambda = line.substring(parts[1], parts[2] - 1).toFloat();
        float phi = line.substring(parts[2], (part_count > 3 ? parts[3] - 1 : line.length())).toFloat();
        
        float psi;
        
        if(mode == MODE_P2P) {
            if(part_count < 5) return;
            
            float lambda_target = line.substring(parts[3], parts[4] - 1).toFloat();
            float phi_target = line.substring(parts[4], line.length()).toFloat();
            
            psi = optimalHeading(lambda, phi, lambda_target, phi_target); //+0.7853981633974483 test error
            
        } else if(mode == MODE_LOITER) {
            if(part_count < 8) return;
            
            float lambda_c = line.substring(parts[3], parts[4] - 1).toFloat();
            float phi_c = line.substring(parts[4], parts[5] - 1).toFloat();
            float R = line.substring(parts[5], parts[6] - 1).toFloat();
            float k = line.substring(parts[6], parts[7] - 1).toFloat();
            int lambda_orbit = line.substring(parts[7], line.length()).toInt();
            
            psi = orbitGuidance(lambda, phi, lambda_c, phi_c, R, k, lambda_orbit);
        }
        
        Serial.println(psi, 9);
    }
}
