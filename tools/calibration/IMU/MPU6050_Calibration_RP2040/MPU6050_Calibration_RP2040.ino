#include <Wire.h>

// I2C Address
#define MPU6050_ADDR 0x68

// Register addresses
#define REG_PWR_MGMT_1    0x6B
#define REG_GYRO_CONFIG   0x1B
#define REG_ACCEL_CONFIG  0x1C
#define REG_CONFIG        0x1A
#define REG_ACCEL_XOUT_H  0x3B

// Configuration values
#define GYRO_RANGE_2000   0x18  // ±2000°/s
#define ACCEL_RANGE_8G    0x10  // ±8g
#define DLPF_260HZ        0x00  // 260Hz bandwidth

// Calibration settings
#define CALIBRATION_SAMPLES 1000
#define CALIBRATION_DURATION_MS 10000

// Raw sensor data
int16_t raw_ax, raw_ay, raw_az;
int16_t raw_gx, raw_gy, raw_gz;

// Accumulated values
int32_t sum_ax = 0, sum_ay = 0, sum_az = 0;
int32_t sum_gx = 0, sum_gy = 0, sum_gz = 0;
int32_t sample_count = 0;

void writeReg(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(MPU6050_ADDR);
    Wire.write(reg);
    Wire.write(value);
    Wire.endTransmission();
}

void readRawData() {
    Wire.beginTransmission(MPU6050_ADDR);
    Wire.write(REG_ACCEL_XOUT_H);
    Wire.endTransmission(false);
    Wire.requestFrom(MPU6050_ADDR, (uint8_t)14);
    
    uint8_t data[14];
    for(int i = 0; i < 14 && Wire.available(); i++) {
        data[i] = Wire.read();
    }
    
    raw_ax = (data[0] << 8) | data[1];
    raw_ay = (data[2] << 8) | data[3];
    raw_az = (data[4] << 8) | data[5];
    raw_gx = (data[8] << 8) | data[9];
    raw_gy = (data[10] << 8) | data[11];
    raw_gz = (data[12] << 8) | data[13];
}

void initMPU6050() {
    Wire.setSDA(0);
    Wire.setSCL(1);
    Wire.begin();
    Wire.setClock(400000);
    
    writeReg(REG_PWR_MGMT_1, 0x00);
    delay(100);
    writeReg(REG_PWR_MGMT_1, 0x01);
    writeReg(REG_GYRO_CONFIG, GYRO_RANGE_2000);
    writeReg(REG_ACCEL_CONFIG, ACCEL_RANGE_8G);
    writeReg(REG_CONFIG, DLPF_260HZ);
}

void printInstructions() {
    Serial.println("========================================");
    Serial.println("     MPU6050 CALIBRATION TOOL");
    Serial.println("========================================");
    Serial.println();
    Serial.println("INSTRUCTIONS:");
    Serial.println("1. Place the board on a flat, stable surface");
    Serial.println("2. Ensure the board is perfectly level");
    Serial.println("3. Do NOT move or touch the board");
    Serial.println("4. Press ENTER to start calibration");
    Serial.println();
    Serial.println("Calibration will run for 10 seconds...");
    Serial.println();
}

void runCalibration() {
    Serial.println("Starting calibration in 3 seconds...");
    delay(1000);
    Serial.println("2...");
    delay(1000);
    Serial.println("1...");
    delay(1000);
    Serial.println("CALIBRATING - DO NOT MOVE!");
    Serial.println();
    
    sum_ax = 0; sum_ay = 0; sum_az = 0;
    sum_gx = 0; sum_gy = 0; sum_gz = 0;
    sample_count = 0;
    
    uint32_t start_time = millis();
    uint32_t last_print = millis();
    
    while (millis() - start_time < CALIBRATION_DURATION_MS) {
        readRawData();
        
        sum_ax += raw_ax;
        sum_ay += raw_ay;
        sum_az += raw_az;
        sum_gx += raw_gx;
        sum_gy += raw_gy;
        sum_gz += raw_gz;
        sample_count++;
        
        // Print progress every second
        if (millis() - last_print >= 1000) {
            Serial.print(".");
            last_print = millis();
        }
        
        delay(10);
    }
    
    Serial.println();
    Serial.println("Calibration complete!");
    Serial.println();
}

void printResults() {
    // Calculate averages
    int16_t avg_ax = sum_ax / sample_count;
    int16_t avg_ay = sum_ay / sample_count;
    int16_t avg_az = sum_az / sample_count;
    int16_t avg_gx = sum_gx / sample_count;
    int16_t avg_gy = sum_gy / sample_count;
    int16_t avg_gz = sum_gz / sample_count;
    
    // Remove gravity from Z-axis (should be ~4096 for 8g range)
    int16_t offset_ax = -avg_ax;
    int16_t offset_ay = -avg_ay;
    int16_t offset_az = -(avg_az - 4096);
    int16_t offset_gx = -avg_gx;
    int16_t offset_gy = -avg_gy;
    int16_t offset_gz = -avg_gz;
    
    Serial.println("========================================");
    Serial.println("     CALIBRATION RESULTS");
    Serial.println("========================================");
    Serial.println();
    Serial.print("Samples collected: ");
    Serial.println(sample_count);
    Serial.println();
    
    Serial.println("Raw averages:");
    Serial.print("  Accel X: "); Serial.println(avg_ax);
    Serial.print("  Accel Y: "); Serial.println(avg_ay);
    Serial.print("  Accel Z: "); Serial.println(avg_az);
    Serial.print("  Gyro  X: "); Serial.println(avg_gx);
    Serial.print("  Gyro  Y: "); Serial.println(avg_gy);
    Serial.print("  Gyro  Z: "); Serial.println(avg_gz);
    Serial.println();
    
    Serial.println("========================================");
    Serial.println("COPY THESE VALUES TO MPU6050.h:");
    Serial.println("========================================");
    Serial.println();
    Serial.print("#define ACCEL_X_OFFSET  ");
    Serial.println(-offset_ax);
    Serial.print("#define ACCEL_Y_OFFSET  ");
    Serial.println(-offset_ay);
    Serial.print("#define ACCEL_Z_OFFSET  ");
    Serial.println(-offset_az);
    Serial.print("#define GYRO_X_OFFSET   ");
    Serial.println(-offset_gx);
    Serial.print("#define GYRO_Y_OFFSET   ");
    Serial.println(-offset_gy);
    Serial.print("#define GYRO_Z_OFFSET   ");
    Serial.println(-offset_gz);
    Serial.println();
    Serial.println("========================================");
}

void setup() {
    Serial.begin(115200);
    while (!Serial) delay(10);
    
    delay(1000);
    
    printInstructions();
    
    // Wait for user input
    while (!Serial.available()) {
        delay(100);
    }
    while (Serial.available()) Serial.read();  // Clear buffer
    
    initMPU6050();
    delay(100);
    
    runCalibration();
    printResults();
    
    Serial.println();
    Serial.println("Calibration finished. You can now close this program.");
}

void loop() {
    // Nothing to do
}
