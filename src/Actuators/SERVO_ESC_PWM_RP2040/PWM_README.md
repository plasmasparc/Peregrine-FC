# PWM_Controller Library

Unified library for controlling ESCs and servos on RP2040.

## Features

- Single interface for ESCs and servos
- Multi-channel support
- Per-channel configuration (range, trim, reverse)
- ESC calibration and arming
- Preset configurations
- Normalized control (-1.0 to 1.0 for servos, 0.0 to 1.0 for ESCs)

## Channel Types

- THROTTLE: ESC control (0.0-1.0)
- RUDDER: Yaw control
- ELEVATOR: Pitch control
- AILERON_LEFT/RIGHT: Roll control
- GENERIC: General purpose

## Basic Usage

### Single ESC
```cpp
PWMController controller(1);
PWMConfig esc = PWMPresets::standardESC(12);
controller.addChannel(0, esc);
controller.calibrateESC(0);
controller.armESC(0);
controller.getChannel(0)->setNormalized(0.5f);
```

### Single Servo
```cpp
PWMController controller(1);
controller.addChannel(0, PWMPresets::standardServo(10));
controller.getChannel(0)->setAngle(90);
controller.getChannel(0)->setNormalized(0.0f);
```

### Multiple Channels
```cpp
PWMController aircraft(5);
aircraft.addChannel(0, PWMPresets::standardESC(12));
aircraft.addChannel(1, PWMPresets::standardServo(10));
aircraft.calibrateESC(0);
aircraft.centerAll();
```

## Configuration

### Standard Presets
- `PWMPresets::standardServo(pin)`: 1000-2000µs
- `PWMPresets::standardESC(pin)`: 1000-2000µs

### Custom Configuration
```cpp
PWMConfig custom = {
    .pin = 10,
    .min_us = 1000,
    .max_us = 2500,
    .center_us = 1500,
    .trim_us = 20,
    .type = ELEVATOR,
    .reversed = false
};
```

### Trim and Reverse
```cpp
channel->setTrim(20);      // +20µs offset
channel->setReversed(true); // Reverse direction
```

## Control Methods

- `setMicroseconds(us)`: Direct pulse width
- `setNormalized(value)`: -1.0 to 1.0 (servo) or 0.0 to 1.0 (ESC)
- `setAngle(degrees)`: 0-180° for servos
- `setCenter()`: Move to center position

## Hardware Config

Pin assignments in examples:
- ESC: GP12
- Servos: GP8-11
- Custom pins via configuration

## ESC Calibration

Required before first use:
1. Max signal (2000µs) for 4 seconds
2. Min signal (1000µs) for 4 seconds
3. ESC learns range
