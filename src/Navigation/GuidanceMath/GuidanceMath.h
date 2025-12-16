#ifndef GUIDANCE_MATH_H
#define GUIDANCE_MATH_H

#include <math.h>

#define WGS84_A 6378137.0f
#define WGS84_F (1.0f / 298.257223563f)
#define WGS84_E2 (WGS84_F * (2.0f - WGS84_F))

#define DEG_TO_RAD (M_PI / 180.0f)
#define RAD_TO_DEG (180.0f / M_PI)

void computeRadii(float phi, float* N, float* M);

void geographicToLocal(float lambda, float phi, float lambda_ref, float phi_ref,
                       float* x, float* y);

void localToGeographic(float x, float y, float lambda_ref, float phi_ref,
                       float* lambda, float* phi);

void velocityToRates(float v, float psi, float phi,
                     float* lambda_dot, float* phi_dot);

float optimalHeading(float lambda_current, float phi_current,
                     float lambda_target, float phi_target);

float orbitGuidance(float lambda, float phi, float lambda_c, float phi_c,
                    float R, float k, int lambda_orbit);

#endif
