#ifndef CONFIG_H
#define CONFIG_H

// LoRa Pins
#define LORA_CS_PIN 5
#define LORA_RST_PIN 14
#define LORA_IRQ_PIN 2

// LoRa Settings
#define LORA_FREQ 433.0
#define LORA_BW 125.0
#define LORA_SF 7
#define LORA_CR 5 // RadioLib represents CR 4/5 as 5

// Serial
#define SERIAL_BAUD 115200

// LEDs
#define STATUS_LED_PIN 2
#define RECV_LED_PIN 4

#endif // CONFIG_H
