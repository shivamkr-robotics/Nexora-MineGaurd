import math
import numpy as np
from collections import defaultdict

class ExplainableDetector:
    """
    Explainable Rate-of-Change + Anomaly Scoring model.
    Replaces the previous LSTM/Isolation Forest model.
    Computes a Severity Score based on tilt rate, RSSI drift, crack displacement, and vibration spikes.
    """
    def __init__(self, window_size=6, w1=1.0, w2=0.5, w3=2.0, w4=1.5):
        self.window_size = window_size
        self.node_history = defaultdict(list)
        
        # Weights for Severity Score
        self.w1 = w1 # tilt_rate
        self.w2 = w2 # rssi_drift
        self.w3 = w3 # crack_disp
        self.w4 = w4 # vib_spike

    def process_reading(self, node_id, tilt_x, tilt_y, rssi, crack_disp, vib_spike):
        """
        Processes a single sensor reading and computes the severity score and risk level.
        Returns a dict containing 'risk_level' and 'severity_score'.
        """
        history = self.node_history[node_id]
        
        # Current reading dictionary
        reading = {
            "tilt_mag": math.sqrt(tilt_x**2 + tilt_y**2),
            "rssi": rssi if rssi is not None else -50,
            "crack_disp": crack_disp if crack_disp is not None else 0.0,
            "vib_spike": vib_spike if vib_spike is not None else 0.0
        }
        
        history.append(reading)
        
        # Keep a rolling window of specified size
        if len(history) > self.window_size:
            history.pop(0)
            
        # Not enough history for rate-of-change analysis
        if len(history) < 2:
            return {"risk_level": "Low", "severity_score": 0.0}
            
        # Calculate rates (deltas) for the window
        tilt_rates = []
        rssi_drifts = []
        for i in range(1, len(history)):
            tilt_rates.append(history[i]["tilt_mag"] - history[i-1]["tilt_mag"])
            rssi_drifts.append(history[i]["rssi"] - history[i-1]["rssi"])
            
        # Extract the most recent rates
        latest_tilt_rate = tilt_rates[-1]
        latest_rssi_drift = rssi_drifts[-1]
        
        # Compute z-scores based on the window history
        if len(tilt_rates) > 1:
            mean_tr = np.mean(tilt_rates)
            std_tr = np.std(tilt_rates) + 1e-6 # Add epsilon to avoid div by zero
            z_tilt = abs(latest_tilt_rate - mean_tr) / std_tr
            
            mean_rd = np.mean(rssi_drifts)
            std_rd = np.std(rssi_drifts) + 1e-6
            z_rssi = abs(latest_rssi_drift - mean_rd) / std_rd
        else:
            z_tilt = 0.0
            z_rssi = 0.0
            
        latest_crack = reading["crack_disp"]
        latest_vib = reading["vib_spike"]
        
        # Compute final Severity Score
        severity_score = (self.w1 * z_tilt) + (self.w2 * z_rssi) + (self.w3 * latest_crack) + (self.w4 * latest_vib)
        
        # Determine Risk Level
        if severity_score > 15:
            risk = "Critical"
        elif severity_score > 10:
            risk = "High"
        elif severity_score > 5:
            risk = "Medium"
        else:
            risk = "Low"
            
        return {
            "risk_level": risk, 
            "severity_score": float(severity_score)
        }
