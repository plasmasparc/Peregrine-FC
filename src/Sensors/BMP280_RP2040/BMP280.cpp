#include "BMP280.h"
#include <math.h>

uint8_t BMP280::readReg(uint8_t reg) {
    Wire1.beginTransmission(BMP280_ADDR);
    Wire1.write(reg);
    Wire1.endTransmission(false);
    Wire1.requestFrom(BMP280_ADDR, (uint8_t)1);
    return Wire1.read();
}

void BMP280::writeReg(uint8_t reg, uint8_t value) {
    Wire1.beginTransmission(BMP280_ADDR);
    Wire1.write(reg);
    Wire1.write(value);
    Wire1.endTransmission();
}

void BMP280::readCalibrationData() {
    Wire1.beginTransmission(BMP280_ADDR);
    Wire1.write(REG_CALIB_START);
    Wire1.endTransmission(false);
    Wire1.requestFrom(BMP280_ADDR, (uint8_t)24);
    
    dig_T1 = Wire1.read() | (Wire1.read() << 8);
    dig_T2 = Wire1.read() | (Wire1.read() << 8);
    dig_T3 = Wire1.read() | (Wire1.read() << 8);
    dig_P1 = Wire1.read() | (Wire1.read() << 8);
    dig_P2 = Wire1.read() | (Wire1.read() << 8);
    dig_P3 = Wire1.read() | (Wire1.read() << 8);
    dig_P4 = Wire1.read() | (Wire1.read() << 8);
    dig_P5 = Wire1.read() | (Wire1.read() << 8);
    dig_P6 = Wire1.read() | (Wire1.read() << 8);
    dig_P7 = Wire1.read() | (Wire1.read() << 8);
    dig_P8 = Wire1.read() | (Wire1.read() << 8);
    dig_P9 = Wire1.read() | (Wire1.read() << 8);
}

void BMP280::init() {
    Wire1.setSDA(BMP280_SDA_PIN);
    Wire1.setSCL(BMP280_SCL_PIN);
    Wire1.begin();
    Wire1.setClock(400000);
    
    // Verify chip ID
    uint8_t chip_id = readReg(REG_CHIP_ID);
    if (chip_id != BMP280_CHIP_ID) {
        return;  // Wrong chip
    }
    
    // Read calibration data
    readCalibrationData();
    
    // Configure sensor
    // Config: standby 0.5ms, filter off, SPI disabled
    writeReg(REG_CONFIG, 0x00);
    
    // Control: temp oversampling x2, pressure oversampling x16, normal mode
    writeReg(REG_CTRL_MEAS, (OVERSAMPLING_2X << 5) | (OVERSAMPLING_16X << 2) | MODE_NORMAL);
    
    delay(10);
    
    // Initialize data structure
    baro_data = {0};
}

void BMP280::readRawData(int32_t* raw_temp, int32_t* raw_press) {
    Wire1.beginTransmission(BMP280_ADDR);
    Wire1.write(REG_PRESS_MSB);
    Wire1.endTransmission(false);
    Wire1.requestFrom(BMP280_ADDR, (uint8_t)6);
    
    uint8_t press_msb = Wire1.read();
    uint8_t press_lsb = Wire1.read();
    uint8_t press_xlsb = Wire1.read();
    uint8_t temp_msb = Wire1.read();
    uint8_t temp_lsb = Wire1.read();
    uint8_t temp_xlsb = Wire1.read();
    
    *raw_press = ((uint32_t)press_msb << 12) | ((uint32_t)press_lsb << 4) | ((uint32_t)press_xlsb >> 4);
    *raw_temp = ((uint32_t)temp_msb << 12) | ((uint32_t)temp_lsb << 4) | ((uint32_t)temp_xlsb >> 4);
}

float BMP280::compensateTemperature(int32_t raw_temp) {
    int32_t var1, var2;
    
    var1 = ((((raw_temp >> 3) - ((int32_t)dig_T1 << 1))) * ((int32_t)dig_T2)) >> 11;
    var2 = (((((raw_temp >> 4) - ((int32_t)dig_T1)) * ((raw_temp >> 4) - ((int32_t)dig_T1))) >> 12) * ((int32_t)dig_T3)) >> 14;
    
    t_fine = var1 + var2;
    
    return (t_fine * 5 + 128) >> 8;
}

float BMP280::compensatePressure(int32_t raw_press) {
    int64_t var1, var2, p;
    
    var1 = ((int64_t)t_fine) - 128000;
    var2 = var1 * var1 * (int64_t)dig_P6;
    var2 = var2 + ((var1 * (int64_t)dig_P5) << 17);
    var2 = var2 + (((int64_t)dig_P4) << 35);
    var1 = ((var1 * var1 * (int64_t)dig_P3) >> 8) + ((var1 * (int64_t)dig_P2) << 12);
    var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)dig_P1) >> 33;
    
    if (var1 == 0) {
        return 0;
    }
    
    p = 1048576 - raw_press;
    p = (((p << 31) - var2) * 3125) / var1;
    var1 = (((int64_t)dig_P9) * (p >> 13) * (p >> 13)) >> 25;
    var2 = (((int64_t)dig_P8) * p) >> 19;
    
    p = ((p + var1 + var2) >> 8) + (((int64_t)dig_P7) << 4);
    
    return (float)p / 256.0f;
}

float BMP280::calculateAltitude(float pressure) {
    return 44330.0f * (1.0f - powf(pressure / sea_level_pressure, 0.1903f));
}

void BMP280::update() {
    int32_t raw_temp, raw_press;
    
    readRawData(&raw_temp, &raw_press);
    
    float temp = compensateTemperature(raw_temp);
    baro_data.temperature = temp / 100.0f;
    
    float press = compensatePressure(raw_press);
    baro_data.pressure = press / 100.0f;
    
    baro_data.altitude = calculateAltitude(baro_data.pressure);
}
