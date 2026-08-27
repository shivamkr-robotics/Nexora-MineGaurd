#include "sensors.h"

SensorManager::SensorManager() : dht(DHT_PIN, DHT22) {
    Q_angle = 0.001f;
    Q_bias = 0.003f;
    R_measure = 0.03f;
    
    angleX = 0.0f;
    angleY = 0.0f;
    biasX = 0.0f;
    biasY = 0.0f;
    
    P[0][0] = 0.0f; P[0][1] = 0.0f;
    P[1][0] = 0.0f; P[1][1] = 0.0f;
    
    lastKalmanTime = 0;
}

void SensorManager::init() {
    Wire.begin(I2C_SDA, I2C_SCL);
    
    if (!mpu.begin()) {
        Serial.println("Failed to find MPU6050 chip");
    }
    
    if (!bmp.begin()) {
        Serial.println("Failed to find BMP280 chip");
    }
    
    dht.begin();
    
    pinMode(MQ4_PIN, INPUT);
    pinMode(BATTERY_PIN, INPUT);
}

void SensorManager::calibrate() {
    Serial.println("Calibrating sensors, please wait 2 seconds...");
    delay(2000); // 2 second settle time
    lastKalmanTime = micros();
}

float SensorManager::getKalmanAngle(float newAngle, float newRate, float dt, float& angle, float& bias) {
    float rate = newRate - bias;
    angle += dt * rate;

    P[0][0] += dt * (dt*P[1][1] - P[0][1] - P[1][0] + Q_angle);
    P[0][1] -= dt * P[1][1];
    P[1][0] -= dt * P[1][1];
    P[1][1] += Q_bias * dt;

    float S = P[0][0] + R_measure;
    float K[2];
    K[0] = P[0][0] / S;
    K[1] = P[1][0] / S;

    float y = newAngle - angle;
    angle += K[0] * y;
    bias += K[1] * y;

    float P00_temp = P[0][0];
    float P01_temp = P[0][1];

    P[0][0] -= K[0] * P00_temp;
    P[0][1] -= K[0] * P01_temp;
    P[1][0] -= K[1] * P00_temp;
    P[1][1] -= K[1] * P01_temp;

    return angle;
}

void SensorManager::readSensors(SensorData& data) {
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);
    
    data.accelX = a.acceleration.x;
    data.accelY = a.acceleration.y;
    data.accelZ = a.acceleration.z;
    data.gyroX = g.gyro.x;
    data.gyroY = g.gyro.y;
    data.gyroZ = g.gyro.z;
    
    // Calculate roll and pitch from accel
    float roll  = atan2(data.accelY, data.accelZ) * 180.0 / PI;
    float pitch = atan2(-data.accelX, sqrt(data.accelY * data.accelY + data.accelZ * data.accelZ)) * 180.0 / PI;
    
    uint32_t now = micros();
    float dt = (float)(now - lastKalmanTime) / 1000000.0f;
    lastKalmanTime = now;
    
    data.tiltX = getKalmanAngle(roll, data.gyroX, dt, angleX, biasX);
    data.tiltY = getKalmanAngle(pitch, data.gyroY, dt, angleY, biasY);
    
    data.temperature = bmp.readTemperature();
    data.pressure = bmp.readPressure();
    data.humidity = dht.readHumidity();
    data.dhtTemp = dht.readTemperature();
    
    data.gasLevel = analogRead(MQ4_PIN); // Raw value, can map to ppm
    data.batteryVoltage = (analogRead(BATTERY_PIN) / 4095.0) * 3.3 * 2.0; // Voltage divider
    
    data.crackDisplacement = 0.0f; // Default
    // TODO: implement crack sensor reading using ADC
    // data.crackDisplacement = analogRead(CRACK_SENSOR_PIN) * factor;
    
    detectAnomaly(data);
}

void SensorManager::detectAnomaly(SensorData& data) {
    data.isAlert = false;
    
    if (abs(data.tiltX) > THRESHOLD_TILT || abs(data.tiltY) > THRESHOLD_TILT) {
        data.isAlert = true;
    }
    
    if (data.gasLevel > THRESHOLD_GAS_PPM) { // basic check
        data.isAlert = true;
    }
    
    if (data.temperature > THRESHOLD_TEMP || data.dhtTemp > THRESHOLD_TEMP) {
        data.isAlert = true;
    }
}
