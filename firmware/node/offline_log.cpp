#include "offline_log.h"

void OfflineLogger::init() {
    if (!LittleFS.begin(true)) {
        Serial.println("LittleFS Mount Failed");
        return;
    }
    
    if (!LittleFS.exists("/data")) {
        LittleFS.mkdir("/data");
    }
}

void OfflineLogger::logReading(MeshPacket& packet) {
    File file = LittleFS.open(logFilePath, FILE_APPEND);
    if (!file) {
        Serial.println("Failed to open file for appending");
        return;
    }
    
    if (file.size() > 1024 * 1024) { // 1MB max size
        file.close();
        LittleFS.remove(logFilePath); // start fresh if too large
        file = LittleFS.open(logFilePath, FILE_APPEND);
    }
    
    // timestamp,nodeId,tiltX,tiltY,pressure,temp,humidity,gas,fftFreq,fftAmp,battery
    file.printf("%u,%s,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f\n",
        packet.timestamp,
        packet.nodeId,
        packet.tiltX,
        packet.tiltY,
        packet.pressure,
        packet.temperature,
        packet.humidity,
        packet.gasPpm,
        packet.vibFftFreq,
        packet.vibFftAmp,
        packet.batteryV
    );
    
    file.close();
}

bool OfflineLogger::hasBufferedData() {
    return LittleFS.exists(logFilePath) && LittleFS.open(logFilePath, FILE_READ).size() > 0;
}

int OfflineLogger::getBufferedCount() {
    File file = LittleFS.open(logFilePath, FILE_READ);
    if (!file) return 0;
    
    int count = 0;
    while (file.available()) {
        String line = file.readStringUntil('\n');
        if (line.length() > 5) count++;
    }
    file.close();
    return count;
}

void OfflineLogger::readAndClear(LogCallback callback) {
    File file = LittleFS.open(logFilePath, FILE_READ);
    if (!file) return;
    
    while (file.available()) {
        String line = file.readStringUntil('\n');
        if (line.length() > 5) {
            MeshPacket packet;
            parseCsvLine(line, packet);
            callback(packet);
        }
    }
    file.close();
    LittleFS.remove(logFilePath);
}

void OfflineLogger::parseCsvLine(String line, MeshPacket& packet) {
    // Simple CSV parser for offline flush
    // Format: timestamp,nodeId,tiltX,tiltY,pressure,temp,humidity,gas,fftFreq,fftAmp,battery
    int index = 0;
    String parts[11];
    
    for (unsigned int i = 0; i < line.length(); i++) {
        if (line[i] == ',') {
            index++;
        } else {
            parts[index] += line[i];
        }
    }
    
    packet.timestamp = parts[0].toInt();
    strncpy(packet.nodeId, parts[1].c_str(), 8);
    packet.tiltX = parts[2].toFloat();
    packet.tiltY = parts[3].toFloat();
    packet.pressure = parts[4].toFloat();
    packet.temperature = parts[5].toFloat();
    packet.humidity = parts[6].toFloat();
    packet.gasPpm = parts[7].toFloat();
    packet.vibFftFreq = parts[8].toFloat();
    packet.vibFftAmp = parts[9].toFloat();
    packet.batteryV = parts[10].toFloat();
}

size_t OfflineLogger::getUsedSpace() {
    return LittleFS.usedBytes();
}

size_t OfflineLogger::getTotalSpace() {
    return LittleFS.totalBytes();
}
