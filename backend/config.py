import os
from dotenv import load_dotenv
load_dotenv()

class Config:
    SECRET_KEY = os.environ.get('SECRET_KEY', 'nexora-mineguard-dev-key')
    SQLALCHEMY_DATABASE_URI = os.environ.get('DATABASE_URL', 'sqlite:///mineguard.db')
    SQLALCHEMY_TRACK_MODIFICATIONS = False
    
    # Serial connection to gateway
    SERIAL_PORT = os.environ.get('SERIAL_PORT', 'COM3')  # Change to your gateway's COM port
    SERIAL_BAUD = int(os.environ.get('SERIAL_BAUD', 115200))
    
    # AI Model paths
    LSTM_MODEL_PATH = os.environ.get('LSTM_MODEL_PATH', 'models/lstm_subsidence.keras')
    IFOREST_MODEL_PATH = os.environ.get('IFOREST_MODEL_PATH', 'models/iforest_anomaly.pkl')
    
    # Alert thresholds (can be overridden via API)
    TILT_THRESHOLD = 15.0
    GAS_THRESHOLD_PPM = 1000.0
    VIBRATION_THRESHOLD = 1.5
    TEMP_THRESHOLD = 45.0
