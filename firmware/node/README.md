# Mine Subsidence Monitoring System - ESP32 Node Firmware (SIH26025)

This firmware is for the ESP32 sensor nodes in the ESP-NOW mesh network.

## Configuration
Before flashing, configure the node identity in `config.h`:
1. Open `config.h`.
2. Set `#define NODE_ID "NODE_A"` to the appropriate ID (`NODE_A`, `NODE_B`, `NODE_C`, `NODE_D`).
3. For **Node D only** (the gateway-facing node), uncomment `#define HAS_LORA`.

## Flashing Instructions
1. Open `node.ino` in the Arduino IDE.
2. Select **ESP32 Dev Module** as the board.
3. Install the required libraries (listed below).
4. Connect the ESP32 and click Upload.

## Required Libraries
Install these via the Arduino Library Manager:
- Adafruit MPU6050
- Adafruit BMP280 Library
- DHT sensor library (by Adafruit)
- ArduinoJson
- RadioLib
- arduinoFFT

## Pin Connections
| Component | ESP32 Pin |
|-----------|-----------|
| I2C SDA (MPU+BMP) | 21 |
| I2C SCL (MPU+BMP) | 22 |
| DHT22     | 4 |
| MQ-4      | 34 (Analog) |
| Crack Sens| 35 (Analog - To be implemented) |
| Battery   | 36 (Analog) |
| Buzzer    | 18 |
| LED Status| 2 |
| LED Alert | 19 (Pin 27 if HAS_LORA is defined) |
| LoRa CS   | 5 |
| LoRa RST  | 14 |
| LoRa IRQ  | 26 |
| LoRa MOSI | 23 |
| LoRa MISO | 19 |
| LoRa SCK  | 18 (Conflicts with Buzzer, adjust as needed) |
