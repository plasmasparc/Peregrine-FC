#include "BMP280.h"

BMP280 barometer;

void setup() {
    Serial.begin(115200);
    barometer.init();
    
    // Set sea level pressure for correct altitude
    // barometer.setSeaLevelPressure(1013.25);
}

void loop() {
    // Update barometer data
    barometer.update();
    
    // Option 1: Get full data structure
    const BaroData& data = barometer.getData();
    
    Serial.print("Temperature: ");
    Serial.print(data.temperature, 2);
    Serial.println(" °C");
    
    Serial.print("Pressure: ");
    Serial.print(data.pressure, 2);
    Serial.println(" hPa");
    
    Serial.print("Altitude: ");
    Serial.print(data.altitude, 1);
    Serial.println(" m");
    
    // Option 2: Get individual values
    float temp = barometer.getTemperature();
    float press = barometer.getPressure();
    float alt = barometer.getAltitude();
    
    Serial.println();
    delay(1000);
}
