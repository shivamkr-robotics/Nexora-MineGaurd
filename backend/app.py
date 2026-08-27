import os
from datetime import datetime, timedelta
from flask import Flask, jsonify, request
from flask_cors import CORS
from flask_socketio import SocketIO
from config import Config
from models import db, MeshNode, SensorReading, Alert, FFTSnapshot
from serial_ingester import SerialIngester
from ai_predictor import AIPredictor
from fft_processor import FFTProcessor
from alert_engine import AlertEngine

# Initialize Flask
app = Flask(__name__)
app.config.from_object(Config)
CORS(app)
socketio = SocketIO(app, cors_allowed_origins="*")

# Initialize DB
db.init_app(app)

# Global services
ai_predictor = None
alert_engine = None
serial_ingester = None

def init_services():
    global ai_predictor, alert_engine, serial_ingester
    with app.app_context():
        db.create_all()
        
    ai_predictor = AIPredictor(lstm_path=app.config['LSTM_MODEL_PATH'], iforest_path=app.config['IFOREST_MODEL_PATH'])
    alert_engine = AlertEngine(app.config)
    
    # Start serial ingestion in background
    serial_ingester = SerialIngester(app, socketio, app.config)
    serial_ingester.start()

# --- REST ENDPOINTS ---

@app.route('/api/nodes', methods=['GET'])
def get_nodes():
    nodes = MeshNode.query.all()
    return jsonify([{
        "node_id": n.node_id,
        "display_name": n.display_name,
        "status": n.status,
        "last_seen": n.last_seen.isoformat() if n.last_seen else None,
        "battery_level": n.battery_level
    } for n in nodes])

@app.route('/api/sensors/latest', methods=['GET'])
def get_latest_sensors():
    nodes = MeshNode.query.all()
    latest_readings = []
    for node in nodes:
        reading = SensorReading.query.filter_by(node_id=node.node_id).order_by(SensorReading.timestamp.desc()).first()
        if reading:
            latest_readings.append({
                "node_id": reading.node_id,
                "timestamp": reading.timestamp.isoformat(),
                "tilt_x": reading.tilt_x,
                "tilt_y": reading.tilt_y,
                "pressure": reading.pressure,
                "temperature": reading.temperature,
                "gas_ppm": reading.gas_ppm,
                "risk_level": reading.risk_level
            })
    return jsonify(latest_readings)

@app.route('/api/sensors/history', methods=['GET'])
def get_sensor_history():
    node_id = request.args.get('node_id')
    hours = int(request.args.get('hours', 24))
    
    if not node_id:
        return jsonify({"error": "node_id is required"}), 400
        
    time_threshold = datetime.utcnow() - timedelta(hours=hours)
    readings = SensorReading.query.filter(
        SensorReading.node_id == node_id,
        SensorReading.timestamp >= time_threshold
    ).order_by(SensorReading.timestamp.asc()).all()
    
    return jsonify([{
        "timestamp": r.timestamp.isoformat(),
        "tilt_x": r.tilt_x,
        "tilt_y": r.tilt_y,
        "pressure": r.pressure,
        "temperature": r.temperature,
        "gas_ppm": r.gas_ppm
    } for r in readings])

@app.route('/api/alerts', methods=['GET'])
def get_alerts():
    limit = int(request.args.get('limit', 50))
    alerts = Alert.query.order_by(Alert.timestamp.desc()).limit(limit).all()
    return jsonify([{
        "id": a.id,
        "node_id": a.node_id,
        "type": a.alert_type,
        "severity": a.severity,
        "message": a.message,
        "timestamp": a.timestamp.isoformat(),
        "acknowledged": a.acknowledged
    } for a in alerts])

@app.route('/api/alerts/<int:alert_id>/acknowledge', methods=['POST'])
def acknowledge_alert(alert_id):
    alert = Alert.query.get_or_404(alert_id)
    alert.acknowledged = True
    db.session.commit()
    return jsonify({"success": True})

@app.route('/api/fft/<node_id>', methods=['GET'])
def get_fft(node_id):
    fft_data = FFTProcessor.get_latest_fft(node_id)
    if not fft_data:
        return jsonify({"error": "No FFT data found for node"}), 404
    return jsonify(fft_data)

@app.route('/api/prediction/<node_id>', methods=['GET'])
def get_prediction(node_id):
    # Fetch last 10 readings
    readings = SensorReading.query.filter_by(node_id=node_id).order_by(SensorReading.timestamp.desc()).limit(10).all()
    if len(readings) < 10:
        return jsonify({"error": "Insufficient data for prediction"}), 400
        
    readings.reverse()
    features = []
    for r in readings:
        features.append([r.tilt_x, r.tilt_y, r.pressure, r.temperature, r.humidity, r.gas_ppm, r.vib_fft_freq, r.vib_fft_amp])
        
    prediction = ai_predictor.predict_subsidence(features)
    return jsonify(prediction)

@app.route('/api/system/status', methods=['GET'])
def system_status():
    node_count = MeshNode.query.count()
    online_count = MeshNode.query.filter_by(status='ONLINE').count()
    return jsonify({
        "status": "Healthy",
        "gateway_connected": True, # Managed by serial ingester in practice
        "total_nodes": node_count,
        "online_nodes": online_count,
        "timestamp": datetime.utcnow().isoformat()
    })

# --- WEBSOCKETS ---

@socketio.on('connect')
def handle_connect():
    print("Client connected via WebSocket")

@socketio.on('disconnect')
def handle_disconnect():
    print("Client disconnected")

if __name__ == '__main__':
    # Initialize DB and start background threads
    init_services()
    # Run server
    socketio.run(app, host='0.0.0.0', port=5000, debug=False, use_reloader=False)
