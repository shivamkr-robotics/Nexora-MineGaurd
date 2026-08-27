#ifndef SERIAL_BRIDGE_H
#define SERIAL_BRIDGE_H

#include <Arduino.h>

class SerialBridge {
public:
    // Initialize serial communication
    void init();

    // Forward the LoRa payload along with gateway metadata to Serial
    void forwardPacket(const String& payload, float loraRssi, float loraSNR);

    // Send a periodic heartbeat message
    void sendHeartbeat();
};

#endif // SERIAL_BRIDGE_H
