#ifndef QMC5883_H
#define QMC5883_H

#include <Wire.h>

// I2C configuration
#define QMC5883_ADDR 0x0D
#define QMC5883_SDA_PIN 10
#define QMC5883_SCL_PIN 11

// Register addresses
#define REG_OUT_X_L   0x00
#define REG_OUT_X_M   0x01
#define REG_OUT_Y_L   0x02
#define REG_OUT_Y_M   0x03
#define REG_OUT_Z_L   0x04
#define REG_OUT_Z_M   0x05
#define REG_STATUS    0x06
#define REG_CONFIG_1  0x09
#define REG_CONFIG_2  0x0A

// Configuration values
#define MODE_CONTINUOUS 0x01
#define MODE_STANDBY    0x00

// Magnetometer data structure
struct MagData {
    int16_t x;  // Raw magnetic field X
    int16_t y;  // Raw magnetic field Y
    int16_t z;  // Raw magnetic field Z
    float heading;  // Calculated heading in degrees (0-360)
};

class QMC5883 {
public:
    void init();
    void update();
    
    // Data access
    const MagData& getData() const { return mag_data; }
    
    // Individual getters
    int16_t getX() const { return mag_data.x; }
    int16_t getY() const { return mag_data.y; }
    int16_t getZ() const { return mag_data.z; }
    float getHeading() const { return mag_data.heading; }
    
private:
    MagData mag_data;
    void readRawData();
    void calculateHeading();
};

#endif
