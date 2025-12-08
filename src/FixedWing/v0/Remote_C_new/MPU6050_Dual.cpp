
#include "MPU6050_Dual.h"

static volatile int16_t raw_ax, raw_ay, raw_az, raw_gx, raw_gy, raw_gz;
static volatile float q0 = 1.0f, q1 = 0.0f, q2 = 0.0f, q3 = 0.0f;
static volatile float integralFBx = 0.0f, integralFBy = 0.0f, integralFBz = 0.0f;
static volatile uint32_t last_update = 0;

static volatile float roll_buffer[40];
static volatile float pitch_buffer[40];
static volatile float yaw_buffer[40];
static volatile uint8_t buffer_index = 0;
static volatile bool buffer_ready = false;

static SemaphoreHandle_t data_mutex;

static void writeReg(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(MPU6050_ADDR);
    Wire.write(reg);
    Wire.write(value);
    Wire.endTransmission();
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

static void mahonyUpdate() {
    float ax = (raw_ax - ACCEL_X_OFFSET) / ACCEL_SCALE;
    float ay = (raw_ay - ACCEL_Y_OFFSET) / ACCEL_SCALE;
    float az = (raw_az - ACCEL_Z_OFFSET) / ACCEL_SCALE;
    float gx = ((raw_gx - GYRO_X_OFFSET) / GYRO_SCALE) * DEG_TO_RAD;
    float gy = ((raw_gy - GYRO_Y_OFFSET) / GYRO_SCALE) * DEG_TO_RAD;
    float gz = ((raw_gz - GYRO_Z_OFFSET) / GYRO_SCALE) * DEG_TO_RAD;
    
    uint32_t now = micros();
    float sample_freq = 1000000.0f / (now - last_update);
    last_update = now;
    
    if((ax == 0.0f) && (ay == 0.0f) && (az == 0.0f)) return;
    
    float norm = sqrtf(ax * ax + ay * ay + az * az);
    ax /= norm;
    ay /= norm;
    az /= norm;
    
    float halfvx = q1 * q3 - q0 * q2;
    float halfvy = q0 * q1 + q2 * q3;
    float halfvz = q0 * q0 - 0.5f + q3 * q3;
    
    float halfex = ay * halfvz - az * halfvy;
    float halfey = az * halfvx - ax * halfvz;
    float halfez = ax * halfvy - ay * halfvx;
    
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
    
    float qa = q0, qb = q1, qc = q2;
    
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

static void calculateRPY(float* roll, float* pitch, float* yaw) {
    *yaw = -atan2f(2.0f * (q1 * q2 + q0 * q3), 
                   q0 * q0 + q1 * q1 - q2 * q2 - q3 * q3) * 57.29578f;
    *pitch = asinf(2.0f * (q1 * q3 - q0 * q2)) * 57.29578f;
    *roll = atan2f(2.0f * (q0 * q1 + q2 * q3), 
                   q0 * q0 - q1 * q1 - q2 * q2 + q3 * q3) * 57.29578f;
}

void mpuTask(void* parameter) {
    TickType_t last_wake = xTaskGetTickCount();
    
    while(1) {
        readSensorData();
        mahonyUpdate();
        
        float roll, pitch, yaw;
        calculateRPY(&roll, &pitch, &yaw);
        
        xSemaphoreTake(data_mutex, portMAX_DELAY);
        roll_buffer[buffer_index] = roll;
        pitch_buffer[buffer_index] = pitch;
        yaw_buffer[buffer_index] = yaw;
        buffer_index++;
        if(buffer_index >= 40) {
            buffer_index = 0;
            buffer_ready = true;
        }
        xSemaphoreGive(data_mutex);
        
        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(1));
    }
}

void initMPU6050Dual() {
    Wire.begin(21, 22);
    Wire.setClock(400000);
    
    writeReg(REG_PWR_MGMT_1, 0x00);
    delay(100);
    writeReg(REG_PWR_MGMT_1, 0x01);
    writeReg(REG_GYRO_CONFIG, GYRO_RANGE_2000);
    writeReg(REG_ACCEL_CONFIG, ACCEL_RANGE_8G);
    writeReg(REG_CONFIG, DLPF_260HZ);
    
    last_update = micros();
    
    data_mutex = xSemaphoreCreateMutex();
    
    xTaskCreatePinnedToCore(
        mpuTask,
        "MPU_Task",
        4096,
        NULL,
        1,
        NULL,
        0
    );
}

void getRollPitchYawSmoothed(float* roll, float* pitch, float* yaw) {
    xSemaphoreTake(data_mutex, portMAX_DELAY);
    
    if(!buffer_ready) {
        *roll = 0.0f;
        *pitch = 0.0f;
        *yaw = 0.0f;
        xSemaphoreGive(data_mutex);
        return;
    }
    
    float roll_sum = 0.0f;
    float pitch_sum = 0.0f;
    float yaw_sum = 0.0f;
    
    for(uint8_t i = 0; i < 40; i++) {
        roll_sum += roll_buffer[i];
        pitch_sum += pitch_buffer[i];
        yaw_sum += yaw_buffer[i];
    }
    
    *roll = roll_sum / 40.0f;
    *pitch = pitch_sum / 40.0f;
    *yaw = yaw_sum / 40.0f;
    
    xSemaphoreGive(data_mutex);
}
