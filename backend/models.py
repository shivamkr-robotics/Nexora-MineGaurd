from flask_sqlalchemy import SQLAlchemy
from datetime import datetime
import json

db = SQLAlchemy()

class Mine(db.Model):
    __tablename__ = 'mines'
    id = db.Column(db.Integer, primary_key=True)
    name = db.Column(db.String(100), nullable=False)
    location = db.Column(db.String(255))
    depth = db.Column(db.Float)
    mining_method = db.Column(db.String(100))
    status = db.Column(db.String(50), default='ACTIVE')
    nodes = db.relationship('MeshNode', backref='mine', lazy=True)

    def __repr__(self):
        return f"<Mine {self.name}>"

class MeshNode(db.Model):
    __tablename__ = 'mesh_nodes'
    node_id = db.Column(db.String(50), primary_key=True)
    display_name = db.Column(db.String(100))
    location_desc = db.Column(db.String(255))
    latitude = db.Column(db.Float)
    longitude = db.Column(db.Float)
    status = db.Column(db.String(50), default='OFFLINE') # ONLINE, OFFLINE, ALERT
    last_seen = db.Column(db.DateTime)
    last_rssi = db.Column(db.Integer)
    battery_level = db.Column(db.Float)
    mine_id = db.Column(db.Integer, db.ForeignKey('mines.id'))
    readings = db.relationship('SensorReading', backref='node', lazy=True)
    alerts = db.relationship('Alert', backref='node', lazy=True)
    fft_snapshots = db.relationship('FFTSnapshot', backref='node', lazy=True)

    def __repr__(self):
        return f"<MeshNode {self.node_id}>"

class SensorReading(db.Model):
    __tablename__ = 'sensor_readings'
    id = db.Column(db.Integer, primary_key=True)
    timestamp = db.Column(db.DateTime, default=datetime.utcnow, index=True)
    node_id = db.Column(db.String(50), db.ForeignKey('mesh_nodes.node_id'), nullable=False)
    tilt_x = db.Column(db.Float)
    tilt_y = db.Column(db.Float)
    vibration_magnitude = db.Column(db.Float)
    vib_fft_freq = db.Column(db.Float)
    vib_fft_amp = db.Column(db.Float)
    crack_displacement = db.Column(db.Float, nullable=True)
    rssi = db.Column(db.Integer)
    battery_level = db.Column(db.Float)
    is_anomaly = db.Column(db.Boolean, default=False)
    risk_level = db.Column(db.String(20)) # Low, Medium, High, Critical
    severity_score = db.Column(db.Float, default=0.0)

    def __repr__(self):
        return f"<SensorReading {self.node_id} @ {self.timestamp}>"

class Alert(db.Model):
    __tablename__ = 'alerts'
    id = db.Column(db.Integer, primary_key=True)
    node_id = db.Column(db.String(50), db.ForeignKey('mesh_nodes.node_id'), nullable=False)
    alert_type = db.Column(db.String(50))
    severity = db.Column(db.String(20))
    message = db.Column(db.String(255))
    timestamp = db.Column(db.DateTime, default=datetime.utcnow)
    acknowledged = db.Column(db.Boolean, default=False)
    resolved = db.Column(db.Boolean, default=False)

    def __repr__(self):
        return f"<Alert {self.alert_type} on {self.node_id}>"

class FFTSnapshot(db.Model):
    __tablename__ = 'fft_snapshots'
    id = db.Column(db.Integer, primary_key=True)
    timestamp = db.Column(db.DateTime, default=datetime.utcnow)
    node_id = db.Column(db.String(50), db.ForeignKey('mesh_nodes.node_id'), nullable=False)
    frequency_bins = db.Column(db.Text) # JSON string of array
    amplitude_bins = db.Column(db.Text) # JSON string of array
    dominant_freq = db.Column(db.Float)
    peak_amplitude = db.Column(db.Float)

    def __repr__(self):
        return f"<FFTSnapshot {self.node_id} @ {self.timestamp}>"
