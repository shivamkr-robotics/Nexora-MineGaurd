import json
from models import db, FFTSnapshot
from datetime import datetime, timedelta

class FFTProcessor:
    """
    Server-side FFT utilities for vibration analysis.
    """
    @staticmethod
    def store_fft_snapshot(node_id, freq_bins, amp_bins, dominant_freq, peak_amp):
        """Stores an FFT snapshot to the database."""
        snapshot = FFTSnapshot(
            node_id=node_id,
            frequency_bins=json.dumps(freq_bins),
            amplitude_bins=json.dumps(amp_bins),
            dominant_freq=dominant_freq,
            peak_amplitude=peak_amp
        )
        db.session.add(snapshot)
        db.session.commit()
        return snapshot

    @staticmethod
    def get_latest_fft(node_id):
        """Returns the latest FFT data for a specific node."""
        snapshot = FFTSnapshot.query.filter_by(node_id=node_id).order_by(FFTSnapshot.timestamp.desc()).first()
        if not snapshot:
            return None
        return {
            "id": snapshot.id,
            "node_id": snapshot.node_id,
            "timestamp": snapshot.timestamp.isoformat(),
            "frequency_bins": json.loads(snapshot.frequency_bins),
            "amplitude_bins": json.loads(snapshot.amplitude_bins),
            "dominant_freq": snapshot.dominant_freq,
            "peak_amplitude": snapshot.peak_amplitude
        }

    @staticmethod
    def analyze_vibration_trend(node_id, hours=24):
        """Analyzes dominant frequency trends over time."""
        time_threshold = datetime.utcnow() - timedelta(hours=hours)
        snapshots = FFTSnapshot.query.filter(
            FFTSnapshot.node_id == node_id,
            FFTSnapshot.timestamp >= time_threshold
        ).order_by(FFTSnapshot.timestamp.asc()).all()
        
        if not snapshots:
            return {"trend": "insufficient_data"}
            
        freqs = [s.dominant_freq for s in snapshots]
        if len(freqs) < 2:
            return {"trend": "stable"}
            
        # Simple trend analysis
        start_avg = sum(freqs[:max(1, len(freqs)//4)]) / max(1, len(freqs)//4)
        end_avg = sum(freqs[-max(1, len(freqs)//4):]) / max(1, len(freqs)//4)
        
        trend = "stable"
        if end_avg > start_avg * 1.2:
            trend = "increasing"
        elif end_avg < start_avg * 0.8:
            trend = "decreasing"
            
        return {
            "trend": trend,
            "start_avg": start_avg,
            "end_avg": end_avg,
            "change_pct": ((end_avg - start_avg) / start_avg) * 100 if start_avg else 0
        }
