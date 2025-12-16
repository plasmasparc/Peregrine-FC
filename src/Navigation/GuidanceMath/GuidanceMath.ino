#include "GuidanceMath.h"

void setup() {
    Serial.begin(115200);
    delay(2000);
    
    float phi = 46.7712f * DEG_TO_RAD;
    float lambda = 23.5888f * DEG_TO_RAD;
    
    float N, M;
    computeRadii(phi, &N, &M);
    Serial.print("N: "); Serial.print(N, 2);
    Serial.print(" M: "); Serial.println(M, 2);
    
    float x, y;
    geographicToLocal(lambda, phi, lambda, phi, &x, &y);
    Serial.print("x: "); Serial.print(x, 2);
    Serial.print(" y: "); Serial.println(y, 2);
    
    float lambda_back, phi_back;
    localToGeographic(x, y, lambda, phi, &lambda_back, &phi_back);
    Serial.print("lambda: "); Serial.print(lambda_back * RAD_TO_DEG, 6);
    Serial.print(" phi: "); Serial.println(phi_back * RAD_TO_DEG, 6);
    
    float v = 20.0f;
    float psi = 45.0f * DEG_TO_RAD;
    float lambda_dot, phi_dot;
    velocityToRates(v, psi, phi, &lambda_dot, &phi_dot);
    Serial.print("lambda_dot: "); Serial.print(lambda_dot * RAD_TO_DEG, 9);
    Serial.print(" phi_dot: "); Serial.println(phi_dot * RAD_TO_DEG, 9);
    
    float lambda_target = (23.5888f + 0.01f) * DEG_TO_RAD;
    float phi_target = (46.7712f + 0.01f) * DEG_TO_RAD;
    float psi_optimal = optimalHeading(lambda, phi, lambda_target, phi_target);
    Serial.print("psi_optimal: "); Serial.println(psi_optimal * RAD_TO_DEG, 2);
    
    float lambda_c = lambda;
    float phi_c = phi;
    float R = 500.0f;
    float k = 125.0f;
    int lambda_orbit = 1;
    float psi_orbit = orbitGuidance(lambda + 0.001f, phi, lambda_c, phi_c, R, k, lambda_orbit);
    Serial.print("psi_orbit: "); Serial.println(psi_orbit * RAD_TO_DEG, 2);
}

void loop() {
}
