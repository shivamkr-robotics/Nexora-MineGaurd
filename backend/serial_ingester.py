import threading
import time
import json
import serial
import random
from datetime import datetime
from models import db, MeshNode, SensorReading, Alert

class SerialIngester:
    """
    Reads USB Serial from the gateway and processes data.
    """
    def __init__(self, app, socketio, config, ai_predictor, alert_engine):
        self.app = app
        self.socketio = socketio
        self.config = config
        self.ai_predictor = ai_predictor
        self.alert_engine = alert_engine
        self.running = False
        self.serial_port = config.SERIAL_PORT
        self.serial_baud = config.SERIAL_BAUD

    def start(self):
        """Launches a daemon thread that calls _read_loop"""
        self.running = True
        self.thread = threading.Thread(target=self._read_loop, daemon=True)
        self.thread.start()

    def _read_loop(self):
        """Opens serial port, reads lines, parses JSON, calls _process_packet. Runs mock if fails."""
        try:
            ser = serial.Serial(self.serial_port, self.serial_baud, timeout=2)
            print(f"Connected to serial port {self.serial_port}")
            while self.running:
                try:
                    line = ser.readline().decode('utf-8').strip()
                    if line:
                        self._handle_raw_data(line)
                except Exception as e:
                    print(f"Error reading from serial: {e}")
                    time.sleep(1)
        except serial.SerialException as e:
            print(f"Could not open serial port {self.serial_port}: {e}. Starting mock data generator...")
            self._mock_data_loop()

    def _mock_data_loop(self):
        """Generates mock data every 2 seconds so the dashboard works without hardware"""
        nodes = ['NODE-01', 'NODE-02', 'NODE-03']
        while self.running:
            for node_id in nodes:
                mock_data = {
                    "type": "data",
                    "nodes": [
                        {
                            "node_id": node_id,
                            "tilt_x": random.uniform(-5.0, 5.0),
                            "tilt_y": random.uniform(-5.0, 5.0),
                            "vibration_magnitude": random.uniform(0.1, 0.5),
                            "vib_fft_freq": random.uniform(10.0, 50.0),
                            "vib_fft_amp": random.uniform(0.01, 0.1),
                            "crack_displacement": random.uniform(0.0, 2.0),
                            "rssi": random.randint(-90, -40),
                            "battery_level": random.uniform(3.5, 4.2)
                        }
                    ]
                }
                self._handle_raw_data(json.dumps(mock_data))
            time.sleep(2)

    def _handle_raw_data(self, raw_string):
        try:
            data = json.loads(raw_string)
            if data.get("type") == "heartbeat":
                # Handle heartbeat packets separately - update gateway status (demo mode handles logic)
                pass
            elif data.get("type") == "data":
                self._process_packet(data)
        except json.JSONDecodeError:
            print(f"Invalid JSON received: {raw_string}")
        except Exception as e:
            print(f"Error handling raw data: {e}")

    def _process_packet(self, data):
        """Extracts nodes array, creates SensorReading, updates MeshNode status/last_seen/rssi."""
        with self.app.app_context():
            nodes = data.get("nodes", [])
            for node_data in nodes:
                node_id = node_data.get("node_id")
                if not node_id:
                    continue

                # Ensure node exists
                node = MeshNode.query.get(node_id)
                if not node:
                    node = MeshNode(node_id=node_id, display_name=f"Node {node_id[-2:]}", status="ONLINE")
                    db.session.add(node)
                
                node.last_seen = datetime.utcnow()
                node.last_rssi = node_data.get("rssi")
                node.battery_level = node_data.get("battery_level")
                node.status = "ONLINE"

                reading = SensorReading(
                    node_id=node_id,
                    tilt_x=node_data.get("tilt_x", 0.0),
                    tilt_y=node_data.get("tilt_y", 0.0),
                    vibration_magnitude=node_data.get("vibration_magnitude", 0.0),
                    vib_fft_freq=node_data.get("vib_fft_freq", 0.0),
                    vib_fft_amp=node_data.get("vib_fft_amp", 0.0),
                    crack_displacement=node_data.get("crack_displacement", 0.0),
                    rssi=node_data.get("rssi", 0),
                    battery_level=node_data.get("battery_level", 0.0)
                )

                # Run AI anomaly detection using ExplainableDetector
                prediction = self.ai_predictor.process_reading(
                    node_id=node_id, 
                    tilt_x=reading.tilt_x, 
                    tilt_y=reading.tilt_y, 
                    rssi=reading.rssi, 
                    crack_disp=reading.crack_displacement, 
                    vib_spike=reading.vib_fft_amp
                )
                reading.risk_level = prediction['risk_level']
                reading.severity_score = prediction['severity_score']
                reading.is_anomaly = reading.severity_score > 5.0 # Basic threshold for anomaly flag

                db.session.add(reading)
                db.session.commit()

                # Process Alerts
                generated_alerts = self.alert_engine.process_reading(reading, reading.is_anomaly, reading.risk_level)
                for alert in generated_alerts:
                    self.socketio.emit('new_alert', alert)

                # Emit websocket events for sensor data
                self.socketio.emit('sensor_update', {
                    'node_id': node_id,
                    'tilt_x': reading.tilt_x,
                    'tilt_y': reading.tilt_y,
                    'vib_fft_freq': reading.vib_fft_freq,
                    'vib_fft_amp': reading.vib_fft_amp,
                    'crack_displacement': reading.crack_displacement,
                    'rssi': reading.rssi,
                    'battery_level': reading.battery_level,
                    'severity_score': reading.severity_score,
                    'risk_level': reading.risk_level,
                    'status': node.status,
                    'timestamp': reading.timestamp.isoformat()
                })
