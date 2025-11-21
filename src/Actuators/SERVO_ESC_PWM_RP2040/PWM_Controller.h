#ifndef PWM_CONTROLLER_H
#define PWM_CONTROLLER_H

#include <Servo.h>

// Channel types
enum ChannelType {
    THROTTLE,      // ESC control
    RUDDER,        // Yaw servo
    ELEVATOR,      // Pitch servo
    AILERON_LEFT,  // Roll servo left
    AILERON_RIGHT, // Roll servo right
    GENERIC        // Generic servo/ESC
};

// Channel configuration
struct PWMConfig {
    uint8_t pin;
    uint16_t min_us;      // Minimum pulse width (µs)
    uint16_t max_us;      // Maximum pulse width (µs)
    uint16_t center_us;   // Center position (µs)
    int16_t trim_us;      // Trim offset (µs)
    ChannelType type;
    bool reversed;        // Reverse direction
};

// Single PWM channel
class PWMChannel {
public:
    PWMChannel();
    void init(const PWMConfig& config);
    
    // Control methods
    void setMicroseconds(uint16_t us);
    void setNormalized(float value);  // -1.0 to 1.0 (servo) or 0.0 to 1.0 (ESC)
    void setAngle(uint8_t degrees);   // 0-180° for servos
    void setCenter();
    void disable();
    
    // Configuration
    void setTrim(int16_t trim_us);
    void setReversed(bool reversed);
    
    // Status
    uint16_t getCurrentMicroseconds() const { return current_us; }
    bool isActive() const { return active; }
    const PWMConfig& getConfig() const { return config; }
    
private:
    Servo servo;
    PWMConfig config;
    uint16_t current_us;
    bool active;
    
    uint16_t applyLimits(uint16_t us);
};

// Multi-channel controller
class PWMController {
public:
    PWMController(uint8_t num_channels);
    ~PWMController();
    
    // Channel management
    void addChannel(uint8_t index, const PWMConfig& config);
    PWMChannel* getChannel(uint8_t index);
    
    // Bulk operations
    void setAll(float value);
    void centerAll();
    void disableAll();
    
    // ESC specific
    void calibrateESC(uint8_t index);
    void armESC(uint8_t index);
    
private:
    PWMChannel* channels;
    uint8_t num_channels;
};

// Preset configurations
namespace PWMPresets {
    PWMConfig standardServo(uint8_t pin);
    PWMConfig standardESC(uint8_t pin);
    PWMConfig customServo(uint8_t pin, uint16_t min_us, uint16_t max_us);
    PWMConfig customESC(uint8_t pin, uint16_t min_us, uint16_t max_us);
}

#endif
