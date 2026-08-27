#ifndef MESH_COMM_H
#define MESH_COMM_H

#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>
#include "config.h"

// Packed structure for mesh communication
#pragma pack(push, 1)
struct MeshPacket {
    char nodeId[8];
    uint32_t timestamp;
    float tiltX;
    float tiltY;
    float vibFftFreq;
    float vibFftAmp;
    float crackDisp;
    float batteryV;
    int8_t rssi;
    uint8_t alertFlag;
};
#pragma pack(pop)

class MeshManager {
public:
    void init();
    bool sendData(MeshPacket& packet);
    
    // For Node C aggregation
    MeshPacket* getBufferedPackets();
    int getBufferedCount();
    void clearBuffer();

    static void onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);
    static void onDataReceived(const esp_now_recv_info_t *esp_now_info, const uint8_t *data, int data_len);

private:
    static MeshPacket rxBuffer[10];
    static int rxCount;
};

#endif // MESH_COMM_H
