#include "PWM_Controller.h"

PWMChannel::PWMChannel() : current_us(0), active(false) {}

void PWMChannel::init(const PWMConfig& cfg) {
    config = cfg;
    servo.attach(config.pin, config.min_us, config.max_us);
    active = true;
    setCenter();
}

void PWMChannel::setMicroseconds(uint16_t us) {
    if (!active) return;
    
    int16_t adjusted = us + config.trim_us;
    
    if (adjusted < config.min_us) adjusted = config.min_us;
    if (adjusted > config.max_us) adjusted = config.max_us;
    
    current_us = adjusted;
    servo.writeMicroseconds(adjusted);
}

void PWMChannel::setNormalized(float value) {
    if (!active) return;
    
    if (config.reversed) value = -value;
    
    uint16_t us;
    
    if (config.type == ESC) {
        if (value < 0.0f) value = 0.0f;
        if (value > 1.0f) value = 1.0f;
        
        us = config.min_us + (uint16_t)((config.max_us - config.min_us) * value);
    } else {
        if (value < -1.0f) value = -1.0f;
        if (value > 1.0f) value = 1.0f;
        
        uint16_t center = (config.min_us + config.max_us) / 2;
        uint16_t range = (config.max_us - config.min_us) / 2;
        us = center + (int16_t)(range * value);
    }
    
    setMicroseconds(us);
}

void PWMChannel::setCenter() {
    uint16_t center = (config.min_us + config.max_us) / 2;
    setMicroseconds(center);
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

void PWMController::centerAll() {
    for (uint8_t i = 0; i < num_channels; i++) {
        if (channels[i].isActive() && channels[i].getConfig().type != ESC) {
            channels[i].setCenter();
        }
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
    
    ch->setMicroseconds(2000);
    delay(4000);
    ch->setMicroseconds(1000);
    delay(4000);
}

void PWMController::armESC(uint8_t index) {
    if (index >= num_channels) return;
    
    channels[index].setNormalized(0.0f);
    delay(2000);
}

namespace PWMPresets {
    PWMConfig standardServo(uint8_t pin) {
        return {pin, 1000, 2000, 0, SERVO, false};
    }
    
    PWMConfig standardESC(uint8_t pin) {
        return {pin, 1000, 2000, 0, ESC, false};
    }
    
    PWMConfig customServo(uint8_t pin, uint16_t min_us, uint16_t max_us) {
        return {pin, min_us, max_us, 0, SERVO, false};
    }
    
    PWMConfig customESC(uint8_t pin, uint16_t min_us, uint16_t max_us) {
        return {pin, min_us, max_us, 0, ESC, false};
    }
}
