#include "GuidanceMath.h"

void computeRadii(float phi_rad, float* N_m, float* M_m) {
    float sin_phi = sinf(phi_rad);
    float denom = sqrtf(1.0f - WGS84_E2 * sin_phi * sin_phi);
    
    *N_m = WGS84_A / denom;
    *M_m = WGS84_A * (1.0f - WGS84_E2) / (denom * denom * denom);
}

void geographicToLocal(float lambda_rad, float phi_rad, 
                       float lambda_ref_rad, float phi_ref_rad,
                       float* x_m, float* y_m) {
    float N_m, M_m;
    computeRadii(phi_ref_rad, &N_m, &M_m);
    
    *x_m = N_m * cosf(phi_ref_rad) * (lambda_rad - lambda_ref_rad);
    *y_m = M_m * (phi_rad - phi_ref_rad);
}

void localToGeographic(float x_m, float y_m, 
                       float lambda_ref_rad, float phi_ref_rad,
                       float* lambda_rad, float* phi_rad) {
    float N_m, M_m;
    computeRadii(phi_ref_rad, &N_m, &M_m);
    
    *lambda_rad = lambda_ref_rad + x_m / (N_m * cosf(phi_ref_rad));
    *phi_rad = phi_ref_rad + y_m / M_m;
}

void velocityToRates(float v_mps, float psi_rad, float phi_rad,
                     float* lambda_dot_radps, float* phi_dot_radps) {
    float N_m, M_m;
    computeRadii(phi_rad, &N_m, &M_m);
    
    float v_N_mps = v_mps * cosf(psi_rad);
    float v_E_mps = v_mps * sinf(psi_rad);
    
    *phi_dot_radps = v_N_mps / M_m;
    *lambda_dot_radps = v_E_mps / (N_m * cosf(phi_rad));
}

float optimalHeading(float lambda_current_rad, float phi_current_rad,
                     float lambda_target_rad, float phi_target_rad) {
    float N_m, M_m;
    computeRadii(phi_current_rad, &N_m, &M_m);
    
    float dx_m = N_m * cosf(phi_current_rad) * (lambda_target_rad - lambda_current_rad);
    float dy_m = M_m * (phi_target_rad - phi_current_rad);
    
    return atan2f(dx_m, dy_m);
}

float orbitGuidance(float lambda_rad, float phi_rad, 
                    float lambda_c_rad, float phi_c_rad,
                    float R_m, float k_m, int lambda_orbit) {
    float N_m, M_m;
    computeRadii(phi_rad, &N_m, &M_m);
    
    float x_m = N_m * cosf(phi_rad) * lambda_rad;
    float y_m = M_m * phi_rad;
    float x_c_m = N_m * cosf(phi_rad) * lambda_c_rad;
    float y_c_m = M_m * phi_c_rad;
    
    float dx_m = x_m - x_c_m;
    float dy_m = y_m - y_c_m;
    
    float r_m = sqrtf(dx_m * dx_m + dy_m * dy_m);
    float theta_rad = atan2f(-dx_m, -dy_m);
    
    float psi_rad = theta_rad + lambda_orbit * (atanf((R_m - r_m) / k_m) + M_PI / 2.0f);
    
    return psi_rad;
}
