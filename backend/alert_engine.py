from models import db, Alert

class AlertEngine:
    """
    Evaluates sensor readings and generates alerts based on thresholds and explainable AI scores.
    """
    def __init__(self, config):
        self.config = config

    def _send_mock_twilio_alert(self, alert_type, severity, message):
        """
        Stub logic for Twilio SMS / Email integration.
        """
        print(f"==========================================")
        print(f"MOCK TWILIO SMS SENT: [{severity}] {alert_type} - {message}")
        print(f"==========================================")

    def process_reading(self, reading, anomaly_detected, risk_level):
        """
        Evaluates a SensorReading against thresholds and explainable AI scores.
        Returns a list of alert dictionaries.
        """
        alerts_generated = []
        node_id = reading.node_id

        # Check raw thresholds
        if reading.tilt_x > getattr(self.config, 'TILT_THRESHOLD', 5.0) or reading.tilt_y > getattr(self.config, 'TILT_THRESHOLD', 5.0):
            alerts_generated.append({
                "alert_type": "TILT_WARNING",
                "severity": "High",
                "message": f"Critical tilt detected: X={reading.tilt_x:.1f}, Y={reading.tilt_y:.1f}"
            })

        if reading.vib_fft_freq > 100.0 and reading.vib_fft_amp > getattr(self.config, 'VIBRATION_THRESHOLD', 0.5):
            alerts_generated.append({
                "alert_type": "VIBRATION_WARNING",
                "severity": "High",
                "message": f"Dangerous vibration pattern: {reading.vib_fft_freq:.1f} Hz @ {reading.vib_fft_amp:.2f}g"
            })
            
        if reading.crack_displacement is not None and reading.crack_displacement > 5.0:
            alerts_generated.append({
                "alert_type": "CRACK_SPIKE",
                "severity": "Critical",
                "message": f"Major crack displacement spike: {reading.crack_displacement:.2f} mm"
            })

        # Explainable AI-based alerts
        if reading.severity_score is not None and reading.severity_score > 10:
            severity = "Critical" if reading.severity_score > 15 else "High"
            alerts_generated.append({
                "alert_type": "HIGH_SEVERITY_SCORE",
                "severity": severity,
                "message": f"AI computed high severity score: {reading.severity_score:.2f} (Risk: {risk_level})"
            })

        # Persist alerts and send SMS
        for alert_data in alerts_generated:
            new_alert = Alert(
                node_id=node_id,
                alert_type=alert_data["alert_type"],
                severity=alert_data["severity"],
                message=alert_data["message"]
            )
            db.session.add(new_alert)
            print(f"ALERT GENERATED [{node_id}]: {alert_data['message']}")
            
            # Trigger external notifications for High/Critical alerts
            if alert_data["severity"] in ["High", "Critical"]:
                self._send_mock_twilio_alert(alert_data["alert_type"], alert_data["severity"], alert_data["message"])

        if alerts_generated:
            db.session.commit()

        return alerts_generated
