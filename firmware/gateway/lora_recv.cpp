#include "lora_recv.h"
#include "config.h"
#include <RadioLib.h>
#include <ArduinoJson.h>

// Initialize SX1276 radio module
SX1276 radio = new Module(LORA_CS_PIN, LORA_IRQ_PIN, LORA_RST_PIN);

// Flag to indicate that a packet was received
volatile bool receivedFlag = false;

// Disable interrupt when it's not needed to avoid race conditions
volatile bool enableInterrupt = true;

// ISR function for handling DIO0 interrupt
#if defined(ESP8266) || defined(ESP32)
  ICACHE_RAM_ATTR
#endif
void setFlag(void) {
    if(!enableInterrupt) {
        return;
    }
    receivedFlag = true;
}

bool LoRaReceiver::init() {
    // Initialize radio
    int state = radio.begin(LORA_FREQ, LORA_BW, LORA_SF, LORA_CR);
    if (state != RADIOLIB_ERR_NONE) {
        return false;
    }
    
    // Set the ISR for packet reception
    radio.setDio0Action(setFlag, RISING);
    
    // Start listening for packets
    state = radio.startReceive();
    if (state != RADIOLIB_ERR_NONE) {
        return false;
    }
    return true;
}

bool LoRaReceiver::checkForPacket(String& payload, float& rssi, float& snr) {
    if(receivedFlag) {
        // Disable interrupt to read data safely
        enableInterrupt = false;
        receivedFlag = false;
        
        // Read received data
        int state = radio.readData(payload);
        
        // Read metadata
        rssi = radio.getRSSI();
        snr = radio.getSNR();
        
        // Check if data was successfully read
        if (state == RADIOLIB_ERR_NONE) {
            if(isValidJSON(payload)) {
                // Restart reception and re-enable interrupt
                radio.startReceive();
                enableInterrupt = true;
                return true;
            }
        }
        
        // Restart reception and re-enable interrupt if failed or not JSON
        radio.startReceive();
        enableInterrupt = true;
    }
    return false;
}

bool LoRaReceiver::isValidJSON(const String& payload) {
    JsonDocument doc; 
    DeserializationError error = deserializeJson(doc, payload);
    return !error;
}
