#include "serial_bridge.h"
#include <ArduinoJson.h>

void SerialBridge::init() {
    // Serial is already initialized in gateway.ino, 
    // but this function provides a stub for future extensions.
}

void SerialBridge::forwardPacket(const String& payload, float loraRssi, float loraSNR) {
    JsonDocument doc;
    
    // Add gateway metadata
    doc["gateway_ts"] = millis();
    doc["lora_rssi"] = loraRssi;
    doc["lora_snr"] = loraSNR;
    
    // Attempt to merge the original payload into this document
    JsonDocument payloadDoc;
    DeserializationError error = deserializeJson(payloadDoc, payload);
    
    if (!error) {
        JsonObject obj = payloadDoc.as<JsonObject>();
        for (JsonPair kv : obj) {
            doc[kv.key()] = kv.value();
        }
    } else {
        // Fallback if payload isn't clean JSON
        doc["raw_payload"] = payload;
    }
    
    // Serialize and send over Serial with newline
    serializeJson(doc, Serial);
    Serial.println();
}

void SerialBridge::sendHeartbeat() {
    JsonDocument doc;
    doc["type"] = "heartbeat";
    doc["gateway_ts"] = millis();
    doc["uptime_s"] = millis() / 1000;
    
    serializeJson(doc, Serial);
    Serial.println();
}
