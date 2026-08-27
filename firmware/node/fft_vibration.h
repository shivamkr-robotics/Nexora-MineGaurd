#ifndef FFT_VIBRATION_H
#define FFT_VIBRATION_H

#include <Arduino.h>
#include <arduinoFFT.h>
#include <Adafruit_MPU6050.h>
#include "config.h"

class VibrationFFT {
public:
    VibrationFFT();
    ~VibrationFFT();
    
    void init();
    void collectSamples(Adafruit_MPU6050& mpu);
    void computeFFT();
    
    float getDominantFrequency();
    float getPeakAmplitude();

private:
    double* vReal;
    double* vImag;
    arduinoFFT* FFT;
    float dominantFreq;
    float peakAmp;
    
    unsigned long samplingPeriod;
};

#endif // FFT_VIBRATION_H
