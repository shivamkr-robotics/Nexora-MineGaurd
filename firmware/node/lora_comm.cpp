#include "lora_comm.h"

#ifdef HAS_LORA
#include <SPI.h>

// Initialize radio on defined pins
SX1276 radio = new Module(LORA_CS, LORA_IRQ, LORA_RST);

void LoRaManager::init() {
    SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_CS);
    
    Serial.print(F("[SX1276] Initializing ... "));
    int state = radio.begin(LORA_FREQ, LORA_BW, LORA_SF, LORA_CR);
    if (state == RADIOLIB_ERR_NONE) {
        Serial.println(F("success!"));
    } else {
        Serial.print(F("failed, code "));
        Serial.println(state);
    }
}

bool LoRaManager::sendPacket(const char* json) {
    int state = radio.transmit(json);
    return state == RADIOLIB_ERR_NONE;
}

bool LoRaManager::sendAggregatedData(MeshPacket* packets, int count) {
    StaticJsonDocument<1024> doc;
    JsonArray nodes = doc.createNestedArray("nodes");
    
    for (int i = 0; i < count; i++) {
        JsonObject node = nodes.createNestedObject();
        node["id"] = packets[i].nodeId;
        node["tX"] = packets[i].tiltX;
        node["tY"] = packets[i].tiltY;
        node["p"] = packets[i].pressure;
        node["t"] = packets[i].temperature;
        node["h"] = packets[i].humidity;
        node["g"] = packets[i].gasPpm;
        node["vF"] = packets[i].vibFftFreq;
        node["vA"] = packets[i].vibFftAmp;
        node["c"] = packets[i].crackDisp;
        node["b"] = packets[i].batteryV;
        node["r"] = packets[i].rssi;
        node["a"] = packets[i].alertFlag;
    }
    
    String output;
    serializeJson(doc, output);
    return sendPacket(output.c_str());
}

#endif
