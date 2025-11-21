#include "PWM_Controller.h"

// PWMChannel implementation
PWMChannel::PWMChannel() : current_us(0), active(false) {}

void PWMChannel::init(const PWMConfig& cfg) {
    config = cfg;
    servo.attach(config.pin, config.min_us, config.max_us);
    active = true;
    setCenter();
}

uint16_t PWMChannel::applyLimits(uint16_t us) {
    int16_t adjusted = us + config.trim_us;
    
    if (adjusted < config.min_us) adjusted = config.min_us;
    if (adjusted > config.max_us) adjusted = config.max_us;
    
    return adjusted;
}

void PWMChannel::setMicroseconds(uint16_t us) {
    if (!active) return;
    
    current_us = applyLimits(us);
    servo.writeMicroseconds(current_us);
}

void PWMChannel::setNormalized(float value) {
    if (!active) return;
    
    uint16_t us;
    
    if (config.type == THROTTLE) {
        // ESC: 0.0 to 1.0
        if (value < 0.0f) value = 0.0f;
        if (value > 1.0f) value = 1.0f;
        
        us = config.min_us + (uint16_t)((config.max_us - config.min_us) * value);
    } else {
        // Servo: -1.0 to 1.0
        if (value < -1.0f) value = -1.0f;
        if (value > 1.0f) value = 1.0f;
        
        if (config.reversed) value = -value;
        
        uint16_t range = (config.max_us - config.min_us) / 2;
        us = config.center_us + (int16_t)(range * value);
    }
    
    setMicroseconds(us);
}

void PWMChannel::setAngle(uint8_t degrees) {
    if (!active) return;
    if (degrees > 180) degrees = 180;
    
    uint16_t us = config.min_us + ((config.max_us - config.min_us) * degrees) / 180;
    setMicroseconds(us);
}

void PWMChannel::setCenter() {
    setMicroseconds(config.center_us);
}

void PWMChannel::disable() {
    if (active) {
        servo.detach();
        active = false;
    }
}

void PWMChannel::setTrim(int16_t trim_us) {
    config.trim_us = trim_us;
}

void PWMChannel::setReversed(bool reversed) {
    config.reversed = reversed;
}

// PWMController implementation
PWMController::PWMController(uint8_t num) : num_channels(num) {
    channels = new PWMChannel[num_channels];
}

PWMController::~PWMController() {
    delete[] channels;
}

void PWMController::addChannel(uint8_t index, const PWMConfig& config) {
    if (index >= num_channels) return;
    channels[index].init(config);
}

PWMChannel* PWMController::getChannel(uint8_t index) {
    if (index >= num_channels) return nullptr;
    return &channels[index];
}

void PWMController::setAll(float value) {
    for (uint8_t i = 0; i < num_channels; i++) {
        channels[i].setNormalized(value);
    }
}

void PWMController::centerAll() {
    for (uint8_t i = 0; i < num_channels; i++) {
        channels[i].setCenter();
    }
}

void PWMController::disableAll() {
    for (uint8_t i = 0; i < num_channels; i++) {
        channels[i].disable();
    }
}

void PWMController::calibrateESC(uint8_t index) {
    if (index >= num_channels) return;
    
    PWMChannel* ch = &channels[index];
    const PWMConfig& cfg = ch->getConfig();
    
    ch->setMicroseconds(cfg.max_us);
    delay(4000);
    ch->setMicroseconds(cfg.min_us);
    delay(4000);
}

void PWMController::armESC(uint8_t index) {
    if (index >= num_channels) return;
    
    PWMChannel* ch = &channels[index];
    ch->setNormalized(0.0f);
    delay(2000);
}

// Preset configurations
namespace PWMPresets {
    PWMConfig standardServo(uint8_t pin) {
        return {
            .pin = pin,
            .min_us = 1000,
            .max_us = 2000,
            .center_us = 1500,
            .trim_us = 0,
            .type = GENERIC,
            .reversed = false
        };
    }
    
    PWMConfig standardESC(uint8_t pin) {
        return {
            .pin = pin,
            .min_us = 1000,
            .max_us = 2000,
            .center_us = 1500,
            .trim_us = 0,
            .type = THROTTLE,
            .reversed = false
        };
    }
    
    PWMConfig customServo(uint8_t pin, uint16_t min_us, uint16_t max_us) {
        return {
            .pin = pin,
            .min_us = min_us,
            .max_us = max_us,
            .center_us = (uint16_t)((min_us + max_us) / 2),
            .trim_us = 0,
            .type = GENERIC,
            .reversed = false
        };
    }
    
    PWMConfig customESC(uint8_t pin, uint16_t min_us, uint16_t max_us) {
        return {
            .pin = pin,
            .min_us = min_us,
            .max_us = max_us,
            .center_us = min_us,
            .trim_us = 0,
            .type = THROTTLE,
            .reversed = false
        };
    }
}
