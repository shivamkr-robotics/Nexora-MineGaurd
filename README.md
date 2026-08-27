# Nexora MineGuard ⛏️

**AI-Enabled Low Cost Real Time Mine Subsidence Monitoring, Prediction and Early Warning System for Underground Coal Mines in India**

> **Problem Statement ID:** SIH26025 | **Theme:** Smart Automation / Disaster Management

---

## System Architecture

```
  Underground Mine                           Surface
 ┌──────────────────────────────────────┐   ┌────────────────────────────┐
 │                                      │   │                            │
 │  [NODE_A]──┐                         │   │   ┌──────────┐  USB Serial │
 │  ESP32     │                         │   │   │ GATEWAY  │────────────┐│
 │  +Sensors  ├──ESP-NOW──>[NODE_D]─────LoRa───>│ ESP32    │            ││
 │            │   Mesh     ESP32        │   │   │ +SX1276  │            ││
 │  [NODE_B]──┤            +Sensors     │   │   └──────────┘            ││
 │  ESP32     │            +SX1276      │   │                           ││
 │  +Sensors  │            (LoRa TX)    │   │                     ┌─────┴┴─────┐
 │            │                         │   │                     │   LAPTOP    │
 │  [NODE_C]──┘                         │   │                     │ Flask+SQLite│
 │  ESP32                               │   │                     │ LSTM Model  │
 │  +Sensors                            │   │                     │ Dashboard   │
 │                                      │   │                     └────────────┘
 └──────────────────────────────────────┘   └────────────────────────────┘
```

## Features

| Feature | Description |
|---|---|
| **ESP-NOW Mesh** | 4 sensor nodes communicate peer-to-peer without WiFi router |
| **LoRa Long Range** | Node D bridges underground mesh to surface gateway (433MHz) |
| **Vibration FFT** | 128-sample FFT on accelerometer data → dominant frequency analysis |
| **Kalman-Filtered Tilt** | Fused accelerometer + gyroscope for accurate tilt measurement |
| **RSSI Monitoring** | Link quality tracking for every mesh packet |
| **Offline Logging** | LittleFS flash storage on each node when mesh fails |
| **LSTM Prediction** | Time-series neural network predicts subsidence risk |
| **Anomaly Detection** | Isolation Forest detects unusual sensor patterns |
| **GIS Map** | Leaflet.js interactive map with live node status |
| **Real-time Dashboard** | WebSocket-powered live gauges, charts, and alerts |
| **Alert System** | On-node buzzer + LED, dashboard toasts + sound |

## Sensors Per Node

| Sensor | Measurement | Interface |
|---|---|---|
| MPU6050 | Tilt (X/Y) + Vibration (accel magnitude) | I2C |
| BMP280 | Barometric Pressure | I2C |
| DHT22 | Temperature + Humidity | Digital |
| MQ-4 | Methane Gas (CH4) Concentration | Analog |
| *Crack Sensor* | *Placeholder — coming soon* | *ADC* |

## Tech Stack

- **Firmware:** Arduino (ESP32), ESP-NOW, RadioLib (LoRa SX1276), ArduinoJson, arduinoFFT, LittleFS
- **Backend:** Python Flask, Flask-SocketIO, SQLAlchemy (SQLite), PySerial, TensorFlow/Keras, scikit-learn
- **Frontend:** Vanilla HTML/CSS/JS, Chart.js, Leaflet.js, Socket.IO
- **AI/ML:** LSTM (subsidence prediction), Isolation Forest (anomaly detection)

## Project Structure

```
Nexora-MineGaurd/
├── firmware/
│   ├── node/                    # Sensor node firmware (4 nodes)
│   │   ├── node.ino             # Main sketch
│   │   ├── config.h             # Pin definitions, thresholds, IDs
│   │   ├── sensors.h/.cpp       # Sensor reading + Kalman filter
│   │   ├── mesh_comm.h/.cpp     # ESP-NOW mesh communication
│   │   ├── lora_comm.h/.cpp     # LoRa TX (Node D only)
│   │   ├── fft_vibration.h/.cpp # Vibration FFT analysis
│   │   ├── offline_log.h/.cpp   # LittleFS offline logging
│   │   └── README.md
│   └── gateway/                 # LoRa → Serial bridge
│       ├── gateway.ino
│       ├── config.h
│       ├── lora_recv.h/.cpp     # LoRa RX
│       └── serial_bridge.h/.cpp # JSON Serial output
├── backend/
│   ├── app.py                   # Flask REST API + WebSocket server
│   ├── serial_ingester.py       # USB Serial data reader
│   ├── ai_predictor.py          # LSTM + Isolation Forest
│   ├── fft_processor.py         # FFT data storage/analysis
│   ├── alert_engine.py          # Alert generation
│   ├── models.py                # SQLAlchemy database models
│   ├── config.py                # Configuration
│   └── requirements.txt
├── frontend/
│   ├── index.html               # Dashboard layout
│   ├── css/style.css            # Dark theme styling
│   └── js/
│       ├── app.js               # Main application logic
│       ├── charts.js            # Tilt, vibration, RSSI, prediction charts
│       ├── gis_map.js           # Leaflet.js GIS map
│       ├── fft_chart.js         # FFT bar chart
│       └── alerts.js            # Alert system
├── docs/
│   └── PROJECT_HANDBOOK.md      # Comprehensive project handbook
└── README.md
```

## Quick Start

### Without Hardware (Demo Mode)

The backend automatically generates mock sensor data if no Serial port is connected.

```bash
cd backend
python -m venv venv
venv\Scripts\activate        # Windows
pip install -r requirements.txt
python app.py
```

Open browser → **http://localhost:5000**

### With Hardware

1. **Flash Nodes:** Open `firmware/node/node.ino` in Arduino IDE. Set `NODE_ID` in `config.h` to `"NODE_A"`, flash. Repeat for B, C, D. For Node D, uncomment `#define HAS_LORA`.
2. **Flash Gateway:** Open `firmware/gateway/gateway.ino`, flash to the gateway ESP32.
3. **Connect Gateway:** Plug gateway ESP32 into laptop via USB cable.
4. **Configure COM Port:** Update `SERIAL_PORT` in `backend/config.py` (e.g., `COM3`).
5. **Run Backend:** Follow the steps above.
6. **Open Dashboard:** http://localhost:5000

### Required Arduino Libraries

- Adafruit MPU6050
- Adafruit BMP280
- DHT sensor library
- ArduinoJson (v6)
- RadioLib
- arduinoFFT
- LittleFS (built-in for ESP32)

## Cost Analysis

| Component | Per Node (₹) | Qty | Total (₹) |
|---|---:|---:|---:|
| ESP32 Dev Board | 450 | 5 | 2,250 |
| MPU6050 | 150 | 4 | 600 |
| BMP280 | 180 | 4 | 720 |
| DHT22 | 250 | 4 | 1,000 |
| MQ-4 | 120 | 4 | 480 |
| SX1276 LoRa Module | 350 | 2 | 700 |
| 18650 Battery + Shield | 350 | 4 | 1,400 |
| Enclosure + PCB + Misc | 500 | 5 | 2,500 |
| **Total** | | | **₹ 9,650** |

## Documentation

📖 See [**PROJECT_HANDBOOK.md**](docs/PROJECT_HANDBOOK.md) for the complete project handbook covering architecture, hardware design, communication protocols, firmware, AI/ML, demonstration guide, and more.

## License

This project is developed for Smart India Hackathon 2026 (SIH26025).
