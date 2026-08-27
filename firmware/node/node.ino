#include <Arduino.h>
#include "config.h"
#include "sensors.h"
#include "mesh_comm.h"
#include "fft_vibration.h"
#include "offline_log.h"
#ifdef HAS_LORA
#include "lora_comm.h"
#endif

SensorManager sensorMgr;
MeshManager meshMgr;
VibrationFFT fftAnalyzer;
OfflineLogger offlineLog;

#ifdef HAS_LORA
LoRaManager loraMgr;
#endif

unsigned long lastReadTime = 0;
int readCounter = 0;

void setup() {
    Serial.begin(115200);
    
    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(LED_STATUS, OUTPUT);
    pinMode(LED_ALERT, OUTPUT);
    
    digitalWrite(LED_STATUS, HIGH);
    
    sensorMgr.init();
    sensorMgr.calibrate();
    
    meshMgr.init();
    
#ifdef HAS_LORA
    loraMgr.init();
#endif

    offlineLog.init();
    fftAnalyzer.init();
    
    Serial.println("Node Setup Complete.");
}

// Callback used when flushing offline logs
void sendBufferedPacket(MeshPacket& packet) {
    meshMgr.sendData(packet);
    delay(50); // delay to prevent flooding the mesh
}

void loop() {
    unsigned long now = millis();
    unsigned long interval = NORMAL_READ_INTERVAL;
    
    // Check if we should read faster
    SensorData currentData;
    sensorMgr.detectAnomaly(currentData); // Uses latest values to check alert state
    if (currentData.isAlert) {
        interval = ALERT_READ_INTERVAL;
    }

    if (now - lastReadTime >= interval) {
        lastReadTime = now;
        readCounter++;
        
        sensorMgr.readSensors(currentData);
        
        // Every 5th reading, do FFT analysis
        if (readCounter % 5 == 0) {
            fftAnalyzer.collectSamples(sensorMgr.getMPU());
            fftAnalyzer.computeFFT();
        }
        
        // Build Packet
        MeshPacket packet;
        strncpy(packet.nodeId, NODE_ID, 8);
        packet.timestamp = millis();
        packet.tiltX = currentData.tiltX;
        packet.tiltY = currentData.tiltY;
        packet.pressure = currentData.pressure;
        packet.temperature = currentData.temperature;
        packet.humidity = currentData.humidity;
        packet.gasPpm = currentData.gasLevel;
        packet.crackDisp = currentData.crackDisplacement;
        packet.batteryV = currentData.batteryVoltage;
        packet.vibFftFreq = fftAnalyzer.getDominantFrequency();
        packet.vibFftAmp = fftAnalyzer.getPeakAmplitude();
        packet.alertFlag = currentData.isAlert ? 1 : 0;
        packet.rssi = 0;
        
        if (!meshMgr.sendData(packet)) {
            Serial.println("Mesh send failed. Logging offline.");
            offlineLog.logReading(packet);
        } else if (offlineLog.hasBufferedData()) {
            Serial.println("Flushing offline data...");
            offlineLog.readAndClear(sendBufferedPacket);
        }
        
#ifdef HAS_LORA
        // If this is Node D, forward aggregated data to Gateway
        int count = meshMgr.getBufferedCount();
        if (count > 0) {
            MeshPacket* packets = meshMgr.getBufferedPackets();
            Serial.print("Forwarding aggregated packets: ");
            Serial.println(count);
            loraMgr.sendAggregatedData(packets, count);
            meshMgr.clearBuffer();
        }
#endif

        // Alerts handling
        if (currentData.isAlert || (packet.vibFftAmp > THRESHOLD_VIB_G)) {
            digitalWrite(BUZZER_PIN, HIGH);
            digitalWrite(LED_ALERT, HIGH);
        } else {
            digitalWrite(BUZZER_PIN, LOW);
            digitalWrite(LED_ALERT, LOW);
        }
        
        // Status heartbeat
        digitalWrite(LED_STATUS, !digitalRead(LED_STATUS));
        
        Serial.printf("T:%.2f H:%.2f TiltX:%.2f FFTF:%.2f Alert:%d\n", 
            currentData.temperature, currentData.humidity, currentData.tiltX, packet.vibFftFreq, currentData.isAlert);
    }
}
