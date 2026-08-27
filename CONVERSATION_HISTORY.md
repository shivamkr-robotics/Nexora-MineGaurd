# SIH 2026 — PS26025 Conversation History & Technical Evolution Record

**Project Title:** Wireless Surface Mesh Network for Real-Time Mine Subsidence Detection  
**Problem Statement ID:** SIH26025 (Ministry of Coal | Disaster Management | Hardware Track)  
**Date of Record:** August 27, 2026  
**Author/Team Lead:** Shivam Kumar (Hardware Lead) & AI Assistant  

---

## 1. Initial Prompt & Objectives

### User Request
1. Identify and retrieve the official details for problem statement **SIH26025**.
2. Complete the entire technical implementation (Hardware Firmware, Backend API, AI/ML models, Frontend Dashboard, Documentation).
3. Provide an in-depth **Coal Mine Handbook** covering geology, mining methods, subsidence physics, DGMS regulations, sensor technologies, and innovation ideas.

---

## 2. Phase 1 — Problem Discovery & First Architecture Draft

### What was identified
- **Problem Statement Title:** Development of an AI-enabled Low Cost Real Time Mine Subsidence Monitoring, Prediction and Early Warning System for Underground Coal Mines in India.
- **Sponsoring Body:** Ministry of Coal, Government of India.
- **Initial Assumption (Draft 1):** Multi-hazard underground monitoring system using ESP32 with 4 sensors (MPU6050, BMP280, MQ-4 Methane, DHT22) communicating over WiFi/MQTT to a Flask cloud server with an LSTM neural network.

### First Draft Deliverables Created
- Full 15-Chapter **Coal Mine Handbook** (`coal_mine_handbook.md`) detailing coal formation, Bord & Pillar / Longwall mining, subsidence mechanics (Angle of draw, $S_{\max}$, Influence function), DGMS safety standards, and 7 innovation hooks.
- Firmware prototypes for ESP32 with Kalman filter.
- Backend REST API + WebSocket server.
- Control-room Web Dashboard with live gauges & Chart.js.

---

## 3. Phase 2 — Technical Critique, Refinements & User Discussion

### Key Discussion Points & Critical Analysis

#### Q1: Feasibility of 4-Node ESP-NOW Mesh + LoRa Bridge + Gateway to Laptop
* **User Proposal:** 
  - 4 nodes deployed in a mesh.
  - Nodes communicate peer-to-peer via **ESP-NOW (WiFi)**.
  - Node 4 (Bridge Node) equipped with an **SX1278 (Ra-02 433 MHz) LoRa module**.
  - Gateway ESP32 with matching LoRa module receiving data and passing it to the laptop.
* **Technical Verdict:** 100% viable and practical for hackathon / internal college rounds. ESP-NOW provides $<1\text{ ms}$ latency and requires no router, while LoRa covers the long-range surface-to-gateway link.

#### Q2: Sensor Selection — Why MPU6050 + Flex Sensor only? (Dropping BMP280, MQ-4, DHT22)
* **User's Insight:** If nodes are buried in or positioned on the surface ground above underground mine panels, atmospheric sensors like BMP280 (barometric pressure), MQ-4 (methane), and DHT22 (humidity) are irrelevant to *subsidence movement* and consume unnecessary power (MQ-4 heater alone pulls $\sim 150\text{ mA}$).
* **Technical Verdict:**
  - **MPU6050:** Measures ground tilt & vibration (core subsidence indicators).
  - **Flex Sensor:** Measures structural strain/bending across shear zones or on anchor stakes.
  - **BMP280 / MQ-4 / DHT22:** Dropped for subsidence nodes; kept only as theoretical mine-shaft safety features.

#### Q3: Detailed PS Discovery & Alignment
* The user provided the official detailed text of PS26025:
  > *"The problem envisages development of an AI-enabled smart mine subsidence monitoring and early warning platform based on a **localized wireless surface mesh sensor network deployed above underground mine panels**."*
  > *Core Innovation Hook: 'Wireless Surface Mesh Network for Real Time Subsidence Detection'*
* **Key Realization:** The user's hardware architecture (Surface mesh nodes + ESP-NOW + LoRa bridge) matched the exact intent of the problem statement far closer than a generic underground gas-monitoring setup.

---

## 4. Phase 3 — Comprehensive Comparison: Team Blueprint PDF vs. Implementation Plan

The user provided the official 25-page team blueprint PDF (*"Wireless Surface Mesh Network for Real-Time Mine Subsidence Detection"*). A line-by-line comparison was conducted:

### Comparison Matrix

| Aspect | Team Blueprint PDF | Original AI Plan | Final Agreed Direction |
| :--- | :--- | :--- | :--- |
| **Deployment Location** | Surface ground above mine panels | Underground galleries | **Surface ground above panels** |
| **Node Topology** | 3-4 Nodes with ESP-NOW Mesh + 1 LoRa Bridge | Direct WiFi to Access Point | **ESP-NOW Mesh + LoRa Bridge** |
| **Sensors per Node** | MPU6050 + Flex Sensor + DIY Copper Crack Strip | MPU6050 + BMP280 + MQ4 + DHT22 | **MPU6050 + Flex (+ Crack trigger)** |
| **Relative Distance Metric** | ESP-NOW Packet **RSSI tracking** | Not included | **RSSI-based drift detection** (Zero hardware cost) |
| **Gateway Ingestion** | LoRa $\rightarrow$ ESP32 $\rightarrow$ HTTP POST / Serial $\rightarrow$ Backend | Direct MQTT over Internet | **LoRa $\rightarrow$ Gateway $\rightarrow$ Ingestion Pipeline** |
| **Offline Capability** | MicroSD card buffering on Gateway | In-memory queue | **Local buffering on Gateway** |
| **AI / Detection Logic** | **Rolling Z-Score + Rate-of-Change** (Explainable) | Heavy LSTM Blackbox | **Explainable Rate-of-Change + Anomaly Scoring** |
| **GIS Mapping** | **Leaflet.js** map with node risk heatmap | Static coordinates table | **Leaflet.js GIS Interactive Map** |
| **Cost per Node** | $\approx ₹1,700 - ₹1,950$ | $₹3,330$ | $\mathbf{\approx ₹1,800}$ (Budget optimized) |

---

## 5. Phase 4 — Final Master Architecture & Consensus

### 1. Hardware Layer (ESP32 Firmware)
- **Node A, Node B, Node D:** ESP32 + MPU6050 (I2C: SDA 21, SCL 22) + Flex Sensor (ADC: GPIO 34) + Crack Trigger (GPIO 25).
- **Node C (Bridge):** Same sensors + **Ra-02 SX1278 LoRa** (SPI: MOSI 23, MISO 19, SCK 18, CS 5, RST 14, DIO0 2).
- **Gateway Node:** ESP32 + LoRa SX1278 + Local Storage / Ingestion link to Laptop.
- **Mesh Protocol:** ESP-NOW broadcast/unicast with RSSI logging.

### 2. Detection & AI Logic
- Real-time rolling window ($W = 30\text{ s}$) computing mean $\mu$ and standard deviation $\sigma$.
- Rate-of-Change ($\frac{\Delta \text{tilt}}{\Delta t}$) to prevent false alarms on naturally sloped terrain.
- Multi-sensor fusion score:
  $$\text{Severity Score} = w_1 \cdot z_{\text{tilt\_rate}} + w_2 \cdot z_{\text{rssi\_drift}} + w_3 \cdot \text{crack\_flag} + w_4 \cdot \text{vib\_spike}$$

### 3. Backend & Dashboard
- **Backend:** Python (FastAPI/Flask) with SQLite storage and WebSocket live broadcasts.
- **Frontend Dashboard:** Dark industrial control-room theme, **Leaflet.js** GIS map rendering node health and deformation heatmaps, **Chart.js** real-time streams, and alert notification panel.
- **Alert Dispatch:** WebSocket instant push + Twilio SMS / Email alerts.

---

## 6. Document References & Workspace Map

- `coal_mine_handbook.md`: 15-Chapter master reference on coal mining, subsidence physics, DGMS regulations, and innovation ideas.
- `implementation_plan.md`: Current sprint roadmap and component status.
- `walkthrough.md`: Verification guide, demo steps, and test procedures.
- `README.md`: High-level project repository documentation.
- `firmware/`: ESP32 Arduino / PlatformIO code for Sensor Nodes, Bridge, and Gateway.
- `backend/`: API server, ingestion service, anomaly detection algorithms.
- `frontend/`: Web dashboard with Leaflet GIS map and live charts.
- `docs/`: Architecture diagrams, Hardware BOM, API documentation, and SIH presentation guide.

---
*End of Conversation & Decision Record — SIH 2026 PS26025*
