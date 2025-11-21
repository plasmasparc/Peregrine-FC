#include "QMC5883.h"
#include <math.h>

void QMC5883::init() {
    // Configure I2C pins
    Wire1.setSDA(QMC5883_SDA_PIN);
    Wire1.setSCL(QMC5883_SCL_PIN);
    Wire1.begin();
    Wire1.setClock(400000);
    
    // Set continuous measurement mode
    Wire1.beginTransmission(QMC5883_ADDR);
    Wire1.write(REG_CONFIG_1);
    Wire1.write(MODE_CONTINUOUS);
    Wire1.endTransmission();
    
    delay(10);
    
    // Initialize data structure
    mag_data = {0};
}

void QMC5883::readRawData() {
    Wire1.beginTransmission(QMC5883_ADDR);
    Wire1.write(REG_OUT_X_L);
    Wire1.endTransmission(false);
    
    Wire1.requestFrom(QMC5883_ADDR, (uint8_t)6);
    
    if (Wire1.available() == 6) {
        uint8_t xl = Wire1.read();
        uint8_t xh = Wire1.read();
        uint8_t yl = Wire1.read();
        uint8_t yh = Wire1.read();
        uint8_t zl = Wire1.read();
        uint8_t zh = Wire1.read();
        
        mag_data.x = (int16_t)(xh << 8 | xl);
        mag_data.y = (int16_t)(yh << 8 | yl);
        mag_data.z = (int16_t)(zh << 8 | zl);
    }
}

void QMC5883::calculateHeading() {
    // Calculate heading from X and Y components
    float heading = atan2f((float)mag_data.y, (float)mag_data.x) * 180.0f / M_PI;
    
    // Normalize to 0-360 degrees
    if (heading < 0) {
        heading += 360.0f;
    }
    
    mag_data.heading = heading;
}

void QMC5883::update() {
    readRawData();
    calculateHeading();
}
