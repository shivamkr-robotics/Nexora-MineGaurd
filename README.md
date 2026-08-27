# Nexora MineGuard (SIH26025)

## Problem Statement: SIH26025
**Development of an AI-enabled Low Cost Real Time Mine Subsidence Monitoring, Prediction and Early Warning System for Underground Coal Mines in India.**

## Architecture Overview
This project implements the Phase 4 Master Architecture consensus: a wireless surface mesh network deployed above underground mine panels.

*   **Sensor Nodes (A, B, D):** ESP32, MPU6050 (Tilt/Vibration), Flex Sensor (Crack Displacement).
*   **Bridge Node (C):** Aggregates ESP-NOW mesh data and forwards it via LoRa (SX1278).
*   **Gateway:** Receives LoRa packets and forwards them to a local backend via USB Serial.
*   **Backend:** Python Flask + SQLite. Uses an **Explainable Z-Score & Rate-of-Change** anomaly detection model.
*   **Frontend:** Leaflet.js GIS map, Chart.js real-time streams, and WebSocket alerts.

## Hardware Pinouts

| Component | Pin (ESP32) |
| :--- | :--- |
| MPU6050 SDA | 21 |
| MPU6050 SCL | 22 |
| Flex Sensor (ADC) | 34 |
| Buzzer | 15 |
| Status LED | 2 |
| LoRa MOSI (Node C/GW) | 23 |
| LoRa MISO (Node C/GW) | 19 |
| LoRa SCK (Node C/GW) | 18 |
| LoRa CS (Node C/GW) | 5 |
| LoRa RST (Node C/GW) | 14 |
| LoRa DIO0 (Node C/GW) | 2 |

## Quick Start (Demo Mode)
You can run the dashboard with synthetic data without any hardware connected:
1. cd backend
2. python -m venv venv
3. .\venv\Scripts\activate
4. pip install -r requirements.txt
5. python app.py
6. Open http://localhost:5000 in your browser.
