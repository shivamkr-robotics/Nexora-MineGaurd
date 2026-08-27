#ifndef LORA_RECV_H
#define LORA_RECV_H

#include <Arduino.h>

class LoRaReceiver {
public:
    // Initializes the LoRa radio with configured settings
    bool init();

    // Non-blocking check to see if a packet is available.
    // If available, fills payload, rssi, and snr and returns true.
    bool checkForPacket(String& payload, float& rssi, float& snr);

private:
    // Internal helper to validate if the payload is JSON
    bool isValidJSON(const String& payload);
};

#endif // LORA_RECV_H
