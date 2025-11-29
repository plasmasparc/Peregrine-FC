#include "MPU6050.h"

// Private variables
static int16_t raw_ax, raw_ay, raw_az, raw_gx, raw_gy, raw_gz;
static float ax, ay, az, gx, gy, gz;
static float q0 = 1.0f, q1 = 0.0f, q2 = 0.0f, q3 = 0.0f;
static float integralFBx = 0.0f, integralFBy = 0.0f, integralFBz = 0.0f;
static uint32_t last_update = 0;
static float sample_freq = 0.0f;

// Private functions
static void writeReg(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(MPU6050_ADDR);
    Wire.write(reg);
    Wire.write(value);
    Wire.endTransmission();
}

static uint8_t readReg(uint8_t reg) {
    Wire.beginTransmission(MPU6050_ADDR);
    Wire.write(reg);
    Wire.endTransmission(false);
    Wire.requestFrom(MPU6050_ADDR, (uint8_t)1);
    return Wire.read();
}

static void readRegs(uint8_t reg, uint8_t count, uint8_t* dest) {
    Wire.beginTransmission(MPU6050_ADDR);
    Wire.write(reg);
    Wire.endTransmission(false);
    Wire.requestFrom(MPU6050_ADDR, count);
    for(uint8_t i = 0; i < count && Wire.available(); i++) {
        dest[i] = Wire.read();
    }
}

static void readSensorData() {
    uint8_t data[14];
    readRegs(REG_ACCEL_XOUT_H, 14, data);
    
    raw_ax = (data[0] << 8) | data[1];
    raw_ay = (data[2] << 8) | data[3];
    raw_az = (data[4] << 8) | data[5];
    raw_gx = (data[8] << 8) | data[9];
    raw_gy = (data[10] << 8) | data[11];
    raw_gz = (data[12] << 8) | data[13];
}

static void convertToPhysical() {
    ax = (raw_ax - ACCEL_X_OFFSET) / ACCEL_SCALE;
    ay = (raw_ay - ACCEL_Y_OFFSET) / ACCEL_SCALE;
    az = (raw_az - ACCEL_Z_OFFSET) / ACCEL_SCALE;
    gx = ((raw_gx - GYRO_X_OFFSET) / GYRO_SCALE) * DEG_TO_RAD;
    gy = ((raw_gy - GYRO_Y_OFFSET) / GYRO_SCALE) * DEG_TO_RAD;
    gz = ((raw_gz - GYRO_Z_OFFSET) / GYRO_SCALE) * DEG_TO_RAD;
}

static void mahonyUpdate() {
    float norm, halfvx, halfvy, halfvz, halfex, halfey, halfez;
    float qa, qb, qc;
    
    uint32_t now = micros();
    sample_freq = 1000000.0f / (now - last_update);
    last_update = now;
    
    if((ax == 0.0f) && (ay == 0.0f) && (az == 0.0f)) return;
    
    norm = sqrtf(ax * ax + ay * ay + az * az);
    ax /= norm;
    ay /= norm;
    az /= norm;
    
    halfvx = q1 * q3 - q0 * q2;
    halfvy = q0 * q1 + q2 * q3;
    halfvz = q0 * q0 - 0.5f + q3 * q3;
    
    halfex = ay * halfvz - az * halfvy;
    halfey = az * halfvx - ax * halfvz;
    halfez = ax * halfvy - ay * halfvx;
    
    if(TWO_KI > 0.0f) {
        integralFBx += TWO_KI * halfex * (1.0f / sample_freq);
        integralFBy += TWO_KI * halfey * (1.0f / sample_freq);
        integralFBz += TWO_KI * halfez * (1.0f / sample_freq);
        gx += integralFBx;
        gy += integralFBy;
        gz += integralFBz;
    }
    
    gx += TWO_KP * halfex;
    gy += TWO_KP * halfey;
    gz += TWO_KP * halfez;
    
    gx *= 0.5f * (1.0f / sample_freq);
    gy *= 0.5f * (1.0f / sample_freq);
    gz *= 0.5f * (1.0f / sample_freq);
    
    qa = q0;
    qb = q1;
    qc = q2;
    
    q0 += (-qb * gx - qc * gy - q3 * gz);
    q1 += (qa * gx + qc * gz - q3 * gy);
    q2 += (qa * gy - qb * gz + q3 * gx);
    q3 += (qa * gz + qb * gy - qc * gx);
    
    norm = sqrtf(q0 * q0 + q1 * q1 + q2 * q2 + q3 * q3);
    q0 /= norm;
    q1 /= norm;
    q2 /= norm;
    q3 /= norm;
}

// Public functions
void initMPU6050() {
    Wire.setSDA(0);
    Wire.setSCL(1);
    Wire.begin();
    Wire.setClock(400000);
    
    writeReg(REG_PWR_MGMT_1, 0x00);  // Wake up
    delay(100);
    writeReg(REG_PWR_MGMT_1, 0x01);  // PLL with X-axis gyro
    writeReg(REG_GYRO_CONFIG, GYRO_RANGE_2000);
    writeReg(REG_ACCEL_CONFIG, ACCEL_RANGE_8G);
    writeReg(REG_CONFIG, DLPF_260HZ);
    
    last_update = micros();
}

void updateMPU6050() {
    readSensorData();
    convertToPhysical();
    mahonyUpdate();
}

void getRollPitchYaw(float* roll, float* pitch, float* yaw) {
    *yaw = -atan2f(2.0f * (q1 * q2 + q0 * q3), 
                   q0 * q0 + q1 * q1 - q2 * q2 - q3 * q3) * 57.29578f;
    *pitch = asinf(2.0f * (q1 * q3 - q0 * q2)) * 57.29578f;
    *roll = atan2f(2.0f * (q0 * q1 + q2 * q3), 
                   q0 * q0 - q1 * q1 - q2 * q2 + q3 * q3) * 57.29578f;
}

void getQuaternion(float* q0_out, float* q1_out, float* q2_out, float* q3_out) {
    *q0_out = q0;
    *q1_out = q1;
    *q2_out = q2;
    *q3_out = q3;
}

void getBodyRates(float* rate_x, float* rate_y, float* rate_z) {
    *rate_x = gx;
    *rate_y = gy;
    *rate_z = gz;
}
