#include "GuidanceMath.h"

void computeRadii(float phi, float* N, float* M) {
    float sin_phi = sinf(phi);
    float denom = sqrtf(1.0f - WGS84_E2 * sin_phi * sin_phi);
    
    *N = WGS84_A / denom;
    *M = WGS84_A * (1.0f - WGS84_E2) / (denom * denom * denom);
}

void geographicToLocal(float lambda, float phi, float lambda_ref, float phi_ref,
                       float* x, float* y) {
    float N, M;
    computeRadii(phi_ref, &N, &M);
    
    *x = N * cosf(phi_ref) * (lambda - lambda_ref);
    *y = M * (phi - phi_ref);
}

void localToGeographic(float x, float y, float lambda_ref, float phi_ref,
                       float* lambda, float* phi) {
    float N, M;
    computeRadii(phi_ref, &N, &M);
    
    *lambda = lambda_ref + x / (N * cosf(phi_ref));
    *phi = phi_ref + y / M;
}

void velocityToRates(float v, float psi, float phi,
                     float* lambda_dot, float* phi_dot) {
    float N, M;
    computeRadii(phi, &N, &M);
    
    float v_N = v * cosf(psi);
    float v_E = v * sinf(psi);
    
    *phi_dot = v_N / M;
    *lambda_dot = v_E / (N * cosf(phi));
}

float optimalHeading(float lambda_current, float phi_current,
                     float lambda_target, float phi_target) {
    float N, M;
    computeRadii(phi_current, &N, &M);
    
    float dx = N * cosf(phi_current) * (lambda_target - lambda_current);
    float dy = M * (phi_target - phi_current);
    
    return atan2f(dx, dy);
}

float orbitGuidance(float lambda, float phi, float lambda_c, float phi_c,
                    float R, float k, int lambda_orbit) {
    float N, M;
    computeRadii(phi, &N, &M);
    
    float x = N * cosf(phi) * lambda;
    float y = M * phi;
    float x_c = N * cosf(phi) * lambda_c;
    float y_c = M * phi_c;
    
    float dx = x - x_c;
    float dy = y - y_c;
    
    float r = sqrtf(dx * dx + dy * dy);
    float theta = atan2f(-dx, -dy);
    
    float psi = theta + lambda_orbit * (atanf((R - r) / k) + M_PI / 2.0f);
    
    return psi;
}
