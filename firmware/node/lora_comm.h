#ifndef LORA_COMM_H
#define LORA_COMM_H

#include "config.h"

#ifdef HAS_LORA
#include <RadioLib.h>
#include <ArduinoJson.h>
#include "mesh_comm.h"

class LoRaManager {
public:
    void init();
    bool sendPacket(const char* json);
    bool sendAggregatedData(MeshPacket* packets, int count);
private:
    // SX1276 radio module
};
#endif

#endif // LORA_COMM_H
