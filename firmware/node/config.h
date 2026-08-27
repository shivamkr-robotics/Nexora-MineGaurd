#ifndef CONFIG_H
#define CONFIG_H

// Node Identity - change this for each physical node
#define NODE_ID "NODE_A" // Valid options: "NODE_A", "NODE_B", "NODE_C", "NODE_D"

// Define HAS_LORA only for Node D
// #define HAS_LORA 

// Mesh Configuration
#define MESH_CHANNEL 1

// Pin Definitions
#define I2C_SDA 21
#define I2C_SCL 22
#define DHT_PIN 4
#define MQ4_PIN 34
#define BUZZER_PIN 15
#define LED_STATUS 2
#define BATTERY_PIN 36

#ifdef HAS_LORA
  #define LED_ALERT 27 // Conflict with MISO, use 27 instead
  // LoRa Pins
  #define LORA_CS 5
  #define LORA_RST 14
  #define LORA_IRQ 26
  #define LORA_MOSI 23
  #define LORA_MISO 19
  #define LORA_SCK 18

#else
  #define LED_ALERT 19
#endif

// Crack sensor: ADC pin (TODO: Implement crack sensor reading)
// #define CRACK_SENSOR_PIN 35 

// Thresholds
#define THRESHOLD_TILT 15.0
#define THRESHOLD_VIB_G 1.5
#define THRESHOLD_GAS_PPM 1000.0
#define THRESHOLD_TEMP 45.0

// Timing Intervals (ms)
#define NORMAL_READ_INTERVAL 5000
#define ALERT_READ_INTERVAL 1000

// FFT Configuration
#define FFT_SAMPLES 128
#define FFT_SAMPLING_FREQ 200.0

// LoRa Configuration
#define LORA_FREQ 433.0
#define LORA_SF 7
#define LORA_BW 125.0
#define LORA_CR 5 // 4/5

#endif // CONFIG_H
