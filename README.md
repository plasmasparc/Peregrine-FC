# RP2040 Flight Controller

Interrupt-driven flight controller for RC aircraft.

## Core Components

**Sensors (ISR-based, thread-safe)**
- MPU6050 IMU: Mahony filter quaternion integration
- GPS: NMEA parsing (NEO-6/8)
- QMC5883: Magnetometer
- BMP280: Barometer

**Actuators**
- PWM: ESC/servo control
- DSHOT300: ESC via PIO state machines

**Communication**
- LoRa telemetry

## Architecture

Independent hardware timers per sensor manager. Atomic data access. Modular design.

## Target Platform

Phoenix 2000 V2 glider
Custom hardware: quadcopter, plane, aerodynamically-control payload

## Applications

- RC aircraft
- Other applications for the codebase: Terminal guidance, ...

## Features

- TVC systems
- Multi-sensor fusion
- Real-time telemetry

