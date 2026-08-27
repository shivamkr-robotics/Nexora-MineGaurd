#include "config.h"
#include "lora_recv.h"
#include "serial_bridge.h"

LoRaReceiver loraRecv;
SerialBridge serialBridge;

void setup() {
    Serial.begin(SERIAL_BAUD);
    Serial.println("Gateway starting...");
    
    pinMode(STATUS_LED_PIN, OUTPUT);
    pinMode(RECV_LED_PIN, OUTPUT);
    
    // Turn on status LED to indicate power
    digitalWrite(STATUS_LED_PIN, HIGH);
    
    // Initialize LoRa and halt on failure
    if (!loraRecv.init()) {
        Serial.println("LoRa initialization failed!");
        while (true) {
            delay(10);
        }
    }
    
    serialBridge.init();
    
    Serial.println("Gateway ready.");
}

void loop() {
    String payload;
    float rssi, snr;
    
    // Non-blocking check for new packet
    if (loraRecv.checkForPacket(payload, rssi, snr)) {
        // Blink RECV LED
        digitalWrite(RECV_LED_PIN, HIGH);
        
        // Forward JSON via Serial
        serialBridge.forwardPacket(payload, rssi, snr);
        
        delay(50); // Small delay to make blink visible
        digitalWrite(RECV_LED_PIN, LOW);
    }
    
    // Heartbeat every 30s
    static unsigned long lastHB = 0;
    if (millis() - lastHB > 30000) {
        serialBridge.sendHeartbeat();
        lastHB = millis();
    }
}
