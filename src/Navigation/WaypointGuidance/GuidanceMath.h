#ifndef GUIDANCE_MATH_H
#define GUIDANCE_MATH_H

#include <math.h>

#define WGS84_A 6378137.0f
#define WGS84_F (1.0f / 298.257223563f)
#define WGS84_E2 (WGS84_F * (2.0f - WGS84_F))

#define DEG_TO_RAD (M_PI / 180.0f)
#define RAD_TO_DEG (180.0f / M_PI)

void computeRadii(float phi_rad, float* N_m, float* M_m);

void geographicToLocal(float lambda_rad, float phi_rad, 
                       float lambda_ref_rad, float phi_ref_rad,
                       float* x_m, float* y_m);

void localToGeographic(float x_m, float y_m, 
                       float lambda_ref_rad, float phi_ref_rad,
                       float* lambda_rad, float* phi_rad);

void velocityToRates(float v_mps, float psi_rad, float phi_rad,
                     float* lambda_dot_radps, float* phi_dot_radps);

float optimalHeading(float lambda_current_rad, float phi_current_rad,
                     float lambda_target_rad, float phi_target_rad);

float orbitGuidance(float lambda_rad, float phi_rad, 
                    float lambda_c_rad, float phi_c_rad,
                    float R_m, float k_m, int lambda_orbit);

#endif
