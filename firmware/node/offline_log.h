#ifndef OFFLINE_LOG_H
#define OFFLINE_LOG_H

#include <Arduino.h>
#include <LittleFS.h>
#include "mesh_comm.h"

typedef void (*LogCallback)(MeshPacket& packet);

class OfflineLogger {
public:
    void init();
    void logReading(MeshPacket& packet);
    bool hasBufferedData();
    int getBufferedCount();
    void readAndClear(LogCallback callback);
    
    size_t getUsedSpace();
    size_t getTotalSpace();

private:
    const char* logFilePath = "/data/log.csv";
    void parseCsvLine(String line, MeshPacket& packet);
};

#endif // OFFLINE_LOG_H
