#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_BMP280.h>
#include <DHT.h>
#include "config.h"

struct SensorData {
    float accelX;
    float accelY;
    float accelZ;
    float gyroX;
    float gyroY;
    float gyroZ;
    float tiltX; // Kalman filtered
    float tiltY; // Kalman filtered
    float temperature;
    float pressure;
    float humidity;
    float dhtTemp;
    float gasLevel;
    float crackDisplacement;
    float batteryVoltage;
    bool isAlert;
};

class SensorManager {
public:
    SensorManager();
    void init();
    void calibrate();
    void readSensors(SensorData& data);
    void detectAnomaly(SensorData& data);
    Adafruit_MPU6050& getMPU() { return mpu; }

private:
    Adafruit_MPU6050 mpu;
    Adafruit_BMP280 bmp;
    DHT dht;
    
    // Kalman filter variables
    float Q_angle;
    float Q_bias;
    float R_measure;
    
    float angleX, angleY;
    float biasX, biasY;
    float P[2][2];
    
    uint32_t lastKalmanTime;
    
    float getKalmanAngle(float newAngle, float newRate, float dt, float& angle, float& bias);
};

#endif // SENSORS_H
