from models import db, Alert

class AlertEngine:
    """
    Evaluates sensor readings and generates alerts based on thresholds and AI predictions.
    """
    def __init__(self, config):
        self.config = config

    def process_reading(self, reading, anomaly_detected, risk_level):
        """
        Evaluates a SensorReading against thresholds and AI flags.
        Returns a list of alert dictionaries.
        """
        alerts_generated = []
        node_id = reading.node_id

        # Check thresholds
        if reading.tilt_x > self.config.TILT_THRESHOLD or reading.tilt_y > self.config.TILT_THRESHOLD:
            alerts_generated.append({
                "alert_type": "TILT_WARNING",
                "severity": "High",
                "message": f"Critical tilt detected: X={reading.tilt_x:.1f}, Y={reading.tilt_y:.1f}"
            })

        if reading.gas_ppm > self.config.GAS_THRESHOLD_PPM:
            alerts_generated.append({
                "alert_type": "GAS_WARNING",
                "severity": "Critical",
                "message": f"Dangerous gas levels: {reading.gas_ppm:.1f} ppm"
            })
            
        if reading.temperature > self.config.TEMP_THRESHOLD:
            alerts_generated.append({
                "alert_type": "TEMP_WARNING",
                "severity": "Medium",
                "message": f"High temperature: {reading.temperature:.1f} °C"
            })

        # Vibration FFT-based alert
        if reading.vib_fft_freq > 100.0 and reading.vib_fft_amp > self.config.VIBRATION_THRESHOLD:
            alerts_generated.append({
                "alert_type": "VIBRATION_WARNING",
                "severity": "High",
                "message": f"Dangerous vibration pattern: {reading.vib_fft_freq:.1f} Hz @ {reading.vib_fft_amp:.2f}g"
            })

        # AI-based alerts
        if anomaly_detected:
            alerts_generated.append({
                "alert_type": "AI_ANOMALY",
                "severity": "Medium",
                "message": "AI detected anomalous sensor behavior pattern"
            })
            
        if risk_level in ["High", "Critical"]:
            alerts_generated.append({
                "alert_type": "SUBSIDENCE_RISK",
                "severity": risk_level,
                "message": f"LSTM model predicts {risk_level} subsidence risk"
            })

        # Persist alerts
        for alert_data in alerts_generated:
            new_alert = Alert(
                node_id=node_id,
                alert_type=alert_data["type"],
                severity=alert_data["severity"],
                message=alert_data["message"]
            )
            db.session.add(new_alert)
            print(f"ALERT GENERATED [{node_id}]: {alert_data['message']}")

        if alerts_generated:
            db.session.commit()

        return alerts_generated
