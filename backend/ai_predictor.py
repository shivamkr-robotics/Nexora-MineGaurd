import numpy as np
import pandas as pd
from sklearn.ensemble import IsolationForest
import tensorflow as tf
from tensorflow.keras.models import Sequential, load_model
from tensorflow.keras.layers import LSTM, Dense, Dropout
import os

class AIPredictor:
    """
    Handles LSTM sequence predictions and Isolation Forest anomaly detection
    for mine subsidence monitoring.
    """
    def __init__(self, lstm_path='models/lstm_subsidence.keras', iforest_path='models/iforest_anomaly.pkl'):
        self.lstm_path = lstm_path
        self.iforest_path = iforest_path
        self.lstm_model = None
        self.iforest_model = None
        self.feature_cols = ['tilt_x', 'tilt_y', 'pressure', 'temperature', 'humidity', 'gas_ppm', 'vib_fft_freq', 'vib_fft_amp']
        self.seq_length = 10
        self._load_or_train_models()

    def _load_or_train_models(self):
        """Loads models if they exist, otherwise trains dummy models."""
        os.makedirs(os.path.dirname(self.lstm_path), exist_ok=True)
        if os.path.exists(self.lstm_path):
            try:
                self.lstm_model = load_model(self.lstm_path)
                print(f"Loaded LSTM model from {self.lstm_path}")
            except Exception as e:
                print(f"Could not load LSTM: {e}. Training new one.")
                self.train_dummy_models()
        else:
            self.train_dummy_models()

    def train_dummy_models(self):
        """
        Generates REALISTIC synthetic mine data and trains models.
        Normal: tilt ~0-3°, pressure ~1000-1020 hPa, temp ~25-35°C, gas ~0-200 ppm
        Anomalous: tilt >10°, pressure drops, gas spikes
        LSTM = Long Short-Term Memory, a type of neural network that remembers patterns over time.
        """
        print("Training dummy AI models...")
        # Generate synthetic data
        num_samples = 1000
        normal_data = np.random.normal(loc=[1.5, 1.5, 1010, 30, 50, 100, 20, 0.05], 
                                     scale=[1.0, 1.0, 5, 2, 5, 50, 5, 0.02], 
                                     size=(num_samples, 8))
        
        # Train Isolation Forest
        self.iforest_model = IsolationForest(contamination=0.05, random_state=42)
        self.iforest_model.fit(normal_data)
        
        # Prepare sequence data for LSTM
        X, y = [], []
        for i in range(len(normal_data) - self.seq_length):
            X.append(normal_data[i:i+self.seq_length])
            # Target is a dummy risk level (0 to 1) based on tilt
            target = min(1.0, max(0.0, normal_data[i+self.seq_length][0] / 15.0))
            y.append(target)
            
        X = np.array(X)
        y = np.array(y)
        
        # Build LSTM Architecture
        self.lstm_model = Sequential([
            LSTM(64, return_sequences=True, input_shape=(self.seq_length, 8)),
            Dropout(0.2),
            LSTM(32),
            Dense(16, activation='relu'),
            Dense(1, activation='sigmoid')
        ])
        
        self.lstm_model.compile(optimizer='adam', loss='mse')
        self.lstm_model.fit(X, y, epochs=5, batch_size=32, verbose=0)
        self.lstm_model.save(self.lstm_path)
        print("Model training complete and saved.")

    def predict_subsidence(self, sequence):
        """
        Predicts subsidence risk based on a sequence of historical data.
        Returns dict with predicted_displacement, risk_level, confidence.
        """
        if self.lstm_model is None or len(sequence) != self.seq_length:
            return {"predicted_displacement": 0.0, "risk_level": "Unknown", "confidence": 0.0}
            
        seq_array = np.array([sequence])
        prediction = self.lstm_model.predict(seq_array, verbose=0)[0][0]
        
        risk_level = "Low"
        if prediction > 0.8:
            risk_level = "Critical"
        elif prediction > 0.6:
            risk_level = "High"
        elif prediction > 0.4:
            risk_level = "Medium"
            
        return {
            "predicted_displacement": float(prediction * 10), # Dummy scaling
            "risk_level": risk_level,
            "confidence": float(1.0 - abs(0.5 - prediction) * 2) # Dummy confidence
        }

    def detect_anomaly(self, features):
        """
        Uses Isolation Forest to detect if a single reading is anomalous.
        Returns boolean.
        """
        if self.iforest_model is None:
            return False
        # features is a 1D array of 8 elements
        pred = self.iforest_model.predict([features])
        return pred[0] == -1
