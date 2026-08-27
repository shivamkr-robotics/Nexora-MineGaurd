#include "sensors.h"

SensorManager::SensorManager() {
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
    
    pinMode(BATTERY_PIN, INPUT);
    pinMode(FLEX_SENSOR_PIN, INPUT);
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
    
    data.batteryVoltage = (analogRead(BATTERY_PIN) / 4095.0) * 3.3 * 2.0; // Voltage divider
    
    // Read flex sensor (ADC 0-4095) for crack displacement simulation
    int flexRaw = analogRead(FLEX_SENSOR_PIN);
    // Simple mapping: assume 0 = 0mm, 4095 = 10.0mm of displacement
    data.crackDisplacement = (flexRaw / 4095.0f) * 10.0f;
    
    detectAnomaly(data);
}

void SensorManager::detectAnomaly(SensorData& data) {
    data.isAlert = false;
    
    if (abs(data.tiltX) > THRESHOLD_TILT || abs(data.tiltY) > THRESHOLD_TILT) {
        data.isAlert = true;
    }
}
