#ifndef DSHOT300_ESC_H
#define DSHOT300_ESC_H

#include <stdint.h>

// Pin configuration for 4 ESCs
#define ESC_PIN_1 2
#define ESC_PIN_2 3
#define ESC_PIN_3 4
#define ESC_PIN_4 5

// Thrust range
#define THRUST_MIN 0.0f
#define THRUST_MAX 1.0f

// Public functions
void escInit();
void escArm();
void escSetThrust(float thrust);
void escSetIndividual(float t1, float t2, float t3, float t4);

#endif
