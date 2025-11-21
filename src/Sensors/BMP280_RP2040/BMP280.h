#ifndef BMP280_H
#define BMP280_H

#include <Wire.h>

// I2C configuration
#define BMP280_ADDR 0x76  // Default address (0x77 if SDO is high)
#define BMP280_SDA_PIN 10
#define BMP280_SCL_PIN 11

// Register addresses
#define REG_CHIP_ID       0xD0
#define REG_RESET         0xE0
#define REG_STATUS        0xF3
#define REG_CTRL_MEAS     0xF4
#define REG_CONFIG        0xF5
#define REG_PRESS_MSB     0xF7
#define REG_PRESS_LSB     0xF8
#define REG_PRESS_XLSB    0xF9
#define REG_TEMP_MSB      0xFA
#define REG_TEMP_LSB      0xFB
#define REG_TEMP_XLSB     0xFC

// Calibration registers
#define REG_CALIB_START   0x88
#define REG_CALIB_END     0xA1

// Chip ID
#define BMP280_CHIP_ID    0x58

// Power modes
#define MODE_SLEEP        0x00
#define MODE_FORCED       0x01
#define MODE_NORMAL       0x03

// Oversampling settings
#define OVERSAMPLING_SKIP 0x00
#define OVERSAMPLING_1X   0x01
#define OVERSAMPLING_2X   0x02
#define OVERSAMPLING_4X   0x03
#define OVERSAMPLING_8X   0x04
#define OVERSAMPLING_16X  0x05

// Barometer data structure
struct BaroData {
    float temperature;  // °C
    float pressure;     // hPa (mbar)
    float altitude;     // meters (relative to sea level)
};

class BMP280 {
public:
    void init();
    void update();
    
    // Data access
    const BaroData& getData() const { return baro_data; }
    
    // Individual getters
    float getTemperature() const { return baro_data.temperature; }
    float getPressure() const { return baro_data.pressure; }
    float getAltitude() const { return baro_data.altitude; }
    
    // Set sea level pressure for altitude calculation (default: 1013.25 hPa)
    void setSeaLevelPressure(float pressure_hpa) { sea_level_pressure = pressure_hpa; }
    
private:
    BaroData baro_data;
    float sea_level_pressure = 1013.25f;
    
    // Calibration data
    uint16_t dig_T1;
    int16_t dig_T2;
    int16_t dig_T3;
    uint16_t dig_P1;
    int16_t dig_P2;
    int16_t dig_P3;
    int16_t dig_P4;
    int16_t dig_P5;
    int16_t dig_P6;
    int16_t dig_P7;
    int16_t dig_P8;
    int16_t dig_P9;
    int32_t t_fine;
    
    void readCalibrationData();
    void readRawData(int32_t* raw_temp, int32_t* raw_press);
    float compensateTemperature(int32_t raw_temp);
    float compensatePressure(int32_t raw_press);
    float calculateAltitude(float pressure);
    uint8_t readReg(uint8_t reg);
    void writeReg(uint8_t reg, uint8_t value);
};

#endif
