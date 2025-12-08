#ifndef PWM_CONTROLLER_H
#define PWM_CONTROLLER_H

#include <Servo.h>

enum ChannelType {
    SERVO,
    ESC
};

struct PWMConfig {
    uint8_t pin;
    uint16_t min_us;
    uint16_t max_us;
    int16_t trim_us;
    ChannelType type;
    bool reversed;
};

class PWMChannel {
public:
    PWMChannel();
    void init(const PWMConfig& cfg);
    
    void setNormalized(float value);
    void setMicroseconds(uint16_t us);
    void setCenter();
    void disable();
    
    void setTrim(int16_t trim_us);
    void setReversed(bool reversed);
    
    uint16_t getCurrentUs() const { return current_us; }
    bool isActive() const { return active; }
    const PWMConfig& getConfig() const { return config; }
    
private:
    Servo servo;
    PWMConfig config;
    uint16_t current_us;
    bool active;
};

class PWMController {
public:
    PWMController(uint8_t num);
    ~PWMController();
    
    void addChannel(uint8_t index, const PWMConfig& config);
    PWMChannel* getChannel(uint8_t index);
    
    void centerAll();
    void disableAll();
    
    void calibrateESC(uint8_t index);
    void armESC(uint8_t index);
    
private:
    PWMChannel* channels;
    uint8_t num_channels;
};

namespace PWMPresets {
    PWMConfig standardServo(uint8_t pin);
    PWMConfig standardESC(uint8_t pin);
    PWMConfig customServo(uint8_t pin, uint16_t min_us, uint16_t max_us);
    PWMConfig customESC(uint8_t pin, uint16_t min_us, uint16_t max_us);
}

#endif
