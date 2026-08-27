// Main App Logic
document.addEventListener('DOMContentLoaded', () => {
    // Clock
    setInterval(() => {
        document.getElementById('clock').innerText = new Date().toLocaleTimeString();
    }, 1000);

    // Canvas Gauges Rendering
    function drawGauge(canvasId, value, min, max, unit, color) {
        const canvas = document.getElementById(canvasId);
        if (!canvas) return;
        const ctx = canvas.getContext('2d');
        const W = canvas.width = canvas.parentElement.clientWidth || 150;
        const H = canvas.height = 80;
        
        ctx.clearRect(0, 0, W, H);
        
        const cx = W / 2;
        const cy = H - 10;
        const r = Math.min(W/2, H) - 15;
        
        // Background arc
        ctx.beginPath();
        ctx.arc(cx, cy, r, Math.PI, 0);
        ctx.lineWidth = 15;
        ctx.strokeStyle = '#2a2a3a';
        ctx.stroke();
        
        // Value arc
        const pct = Math.max(0, Math.min(1, (value - min) / (max - min)));
        const endAngle = Math.PI + (pct * Math.PI);
        
        ctx.beginPath();
        ctx.arc(cx, cy, r, Math.PI, endAngle);
        ctx.lineWidth = 15;
        ctx.strokeStyle = color;
        ctx.stroke();
    }

    let selectedNode = 'NODE_A';
    const nodeSelector = document.getElementById('node-selector');
    nodeSelector.addEventListener('change', (e) => {
        selectedNode = e.target.value;
    });

    const nodeDataStore = {
        'NODE_A': { tilt: 0, vibration: 0, rssi: -70, battery: 100, status: 'safe' },
        'NODE_B': { tilt: 0, vibration: 0, rssi: -70, battery: 100, status: 'safe' },
        'NODE_C': { tilt: 0, vibration: 0, rssi: -70, battery: 100, status: 'safe' },
        'NODE_D': { tilt: 0, vibration: 0, rssi: -70, battery: 100, status: 'safe' }
    };

    function updateNodeList() {
        const list = document.getElementById('node-list');
        list.innerHTML = '';
        
        Object.keys(nodeDataStore).forEach(nodeId => {
            const data = nodeDataStore[nodeId];
            const li = document.createElement('li');
            li.className = `node-item ${data.status}`;
            li.innerHTML = `
                <div class="node-info">
                    <h4>${nodeId}</h4>
                    <div class="node-stats">
                        <span><i class="fa-solid fa-wifi"></i> ${data.rssi} dBm</span>
                        <span><i class="fa-solid fa-battery-half"></i> ${data.battery}%</span>
                    </div>
                </div>
                <div class="node-status-icon">
                    <i class="fa-solid ${data.status === 'safe' ? 'fa-check-circle' : 'fa-exclamation-triangle'}"></i>
                </div>
            `;
            list.appendChild(li);
        });
    }

    // Initialize UI
    updateNodeList();
    drawGauge('gauge-tilt', 0, 0, 90, '°', '#00e5ff');
    drawGauge('gauge-vib', 0, 0, 100, 'Hz', '#00e5ff');
    drawGauge('gauge-crack', 0, 0, 10, 'mm', '#ff3366');

    // Socket.IO
    const socket = io('http://localhost:5000');
    
    socket.on('connect', () => {
        document.querySelector('.status-dot').classList.add('online');
        document.querySelector('.status-dot').style.backgroundColor = '#00e676';
        window.alertsSystem.addAlert('Connected to backend API', 'info');
    });

    socket.on('disconnect', () => {
        document.querySelector('.status-dot').classList.remove('online');
        document.querySelector('.status-dot').style.backgroundColor = '#ff1744';
        window.alertsSystem.addAlert('Disconnected from backend API', 'critical');
    });

    socket.on('sensor_update', (data) => {
        const nodeId = data.node_id;
        if (nodeDataStore[nodeId]) {
            nodeDataStore[nodeId] = { ...nodeDataStore[nodeId], ...data };
        }
        
        // Update charts and gauges if selected
        if (nodeId === selectedNode) {
            // Update Text
            document.getElementById('val-tilt').innerText = data.tilt.toFixed(2);
            document.getElementById('val-vib').innerText = data.vibration.toFixed(2);
            
            // Draw Gauges
            drawGauge('gauge-tilt', data.tilt_x || 0, 0, 45, '°', '#00e5ff');
            drawGauge('gauge-vib', data.vib_fft_amp || 0, 0, 5, 'g', '#ffab00');
            drawGauge('gauge-crack', data.crack_disp || 0, 0, 10, 'mm', '#ff3366');
            
            // Update charts
            window.charts.updateRealtimeCharts(nodeId, data.tilt, data.vibration);
            
            // Mock FFT data for demo based on vibration
            const baseFreq = data.vibration || 10;
            const freqBins = Array.from({length: 20}, (_, i) => i * 5);
            const ampBins = freqBins.map(f => Math.max(0, Math.random() * 5 + (Math.abs(f - baseFreq) < 5 ? 15 : 0)));
            window.fftChart.updateFFT(freqBins, ampBins, baseFreq);
            
            // Mock LSTM Prediction for demo
            const pred = data.tilt + (Math.random() * 2 - 0.5);
            window.charts.updatePrediction(data.tilt, pred);
        }

        window.charts.updateRSSI(nodeId, data.rssi || -70);
        window.gisMap.updateNodeStatus(nodeId, data.status || 'safe', data);
        updateNodeList();
    });

    socket.on('new_alert', (data) => {
        window.alertsSystem.addAlert(data.message, data.type, data.node_id);
    });

    socket.on('node_status', (data) => {
        const nodeId = data.node_id;
        if (nodeDataStore[nodeId]) {
            nodeDataStore[nodeId].status = data.status;
            window.gisMap.updateNodeStatus(nodeId, data.status, nodeDataStore[nodeId]);
            updateNodeList();
        }
    });
});
