#include "mesh_comm.h"

MeshPacket MeshManager::rxBuffer[10];
int MeshManager::rxCount = 0;

void MeshManager::init() {
    WiFi.mode(WIFI_STA);
    if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
        return;
    }

    esp_now_register_send_cb(onDataSent);
    esp_now_register_recv_cb(onDataReceived);

    // Register broadcast peer
    esp_now_peer_info_t peerInfo = {};
    memset(peerInfo.peer_addr, 0xFF, 6);
    peerInfo.channel = MESH_CHANNEL;
    peerInfo.encrypt = false;
    
    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
        Serial.println("Failed to add peer");
    }
}

bool MeshManager::sendData(MeshPacket& packet) {
    uint8_t broadcastMac[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    esp_err_t result = esp_now_send(broadcastMac, (uint8_t *) &packet, sizeof(packet));
    return result == ESP_OK;
}

void MeshManager::onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
    // Serial.print("Last Packet Send Status: ");
    // Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void MeshManager::onDataReceived(const esp_now_recv_info_t *esp_now_info, const uint8_t *data, int data_len) {
    if (data_len == sizeof(MeshPacket)) {
        if (rxCount < 10) {
            memcpy(&rxBuffer[rxCount], data, sizeof(MeshPacket));
            rxBuffer[rxCount].rssi = esp_now_info->rx_ctrl->rssi;
            rxCount++;
        }
    }
}

MeshPacket* MeshManager::getBufferedPackets() {
    return rxBuffer;
}

int MeshManager::getBufferedCount() {
    return rxCount;
}

void MeshManager::clearBuffer() {
    rxCount = 0;
}
