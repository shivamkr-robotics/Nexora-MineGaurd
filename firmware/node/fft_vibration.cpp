#include "fft_vibration.h"

VibrationFFT::VibrationFFT() {
    vReal = new double[FFT_SAMPLES];
    vImag = new double[FFT_SAMPLES];
    FFT = new arduinoFFT(vReal, vImag, FFT_SAMPLES, FFT_SAMPLING_FREQ);
    dominantFreq = 0.0f;
    peakAmp = 0.0f;
}

VibrationFFT::~VibrationFFT() {
    delete[] vReal;
    delete[] vImag;
    delete FFT;
}

void VibrationFFT::init() {
    samplingPeriod = round(1000000.0 / FFT_SAMPLING_FREQ); // microseconds
}

// Collects 128 samples at 200Hz for vibration analysis
void VibrationFFT::collectSamples(Adafruit_MPU6050& mpu) {
    for (int i = 0; i < FFT_SAMPLES; i++) {
        unsigned long t = micros();
        sensors_event_t a, g, temp;
        mpu.getEvent(&a, &g, &temp);
        
        // Calculate magnitude of acceleration
        double magnitude = sqrt(a.acceleration.x * a.acceleration.x + 
                                a.acceleration.y * a.acceleration.y + 
                                a.acceleration.z * a.acceleration.z);
        
        vReal[i] = magnitude;
        vImag[i] = 0.0;
        
        while (micros() - t < samplingPeriod) {
            // wait
        }
    }
}

// Converts time-domain vibration data to frequency-domain to find dominant frequencies
void VibrationFFT::computeFFT() {
    FFT->Windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD); // Apply windowing to reduce leakage
    FFT->Compute(FFT_FORWARD); // Compute FFT
    FFT->ComplexToMagnitude(); // Compute magnitudes
    
    double peak = 0;
    double freq = 0;
    FFT->MajorPeak(&freq, &peak); // Find frequency with most energy
    
    dominantFreq = (float)freq;
    peakAmp = (float)peak;
}

float VibrationFFT::getDominantFrequency() {
    return dominantFreq;
}

float VibrationFFT::getPeakAmplitude() {
    return peakAmp;
}
