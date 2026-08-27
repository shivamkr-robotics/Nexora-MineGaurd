// Charts Logic
(function() {
    Chart.defaults.color = '#8888aa';
    Chart.defaults.font.family = "'Segoe UI', Tahoma, Geneva, Verdana, sans-serif";

    const commonOptions = {
        responsive: true,
        maintainAspectRatio: false,
        animation: false,
        elements: {
            point: { radius: 0 },
            line: { borderWidth: 2 }
        },
        scales: {
            x: { grid: { display: false } },
            y: { grid: { color: '#2a2a3a' } }
        },
        plugins: {
            legend: { labels: { color: '#e0e0e0' } }
        }
    };

    // Tilt Chart
    const ctxTilt = document.getElementById('chart-tilt').getContext('2d');
    const chartTilt = new Chart(ctxTilt, {
        type: 'line',
        data: {
            labels: Array(30).fill(''),
            datasets: [{
                label: 'Tilt (°)',
                data: Array(30).fill(null),
                borderColor: '#00e5ff',
                tension: 0.3
            }]
        },
        options: commonOptions
    });

    // Vibration Chart
    const ctxVib = document.getElementById('chart-vib').getContext('2d');
    const chartVib = new Chart(ctxVib, {
        type: 'line',
        data: {
            labels: Array(30).fill(''),
            datasets: [{
                label: 'Vibration (Hz)',
                data: Array(30).fill(null),
                borderColor: '#ffab00',
                tension: 0.3
            }]
        },
        options: commonOptions
    });

    // RSSI Chart (4 nodes)
    const ctxRssi = document.getElementById('chart-rssi').getContext('2d');
    const rssiDatasets = [
        { label: 'NODE_A', data: Array(30).fill(null), borderColor: '#00e5ff' },
        { label: 'NODE_B', data: Array(30).fill(null), borderColor: '#00e676' },
        { label: 'NODE_C', data: Array(30).fill(null), borderColor: '#ffab00' },
        { label: 'NODE_D', data: Array(30).fill(null), borderColor: '#ff1744' }
    ];
    const chartRssi = new Chart(ctxRssi, {
        type: 'line',
        data: { labels: Array(30).fill(''), datasets: rssiDatasets },
        options: commonOptions
    });

    // Prediction Chart
    const ctxPred = document.getElementById('chart-prediction').getContext('2d');
    const chartPred = new Chart(ctxPred, {
        type: 'line',
        data: {
            labels: Array(20).fill(''),
            datasets: [
                {
                    label: 'Actual',
                    data: Array(20).fill(null),
                    borderColor: '#00e676',
                    tension: 0.1
                },
                {
                    label: 'Predicted',
                    data: Array(20).fill(null),
                    borderColor: '#ff1744',
                    borderDash: [5, 5],
                    tension: 0.1
                }
            ]
        },
        options: commonOptions
    });

    window.charts = {
        updateRealtimeCharts: function(nodeId, tilt, vibration) {
            // We just append data to the end and shift
            const timeLabel = new Date().toLocaleTimeString().split(' ')[0];
            
            chartTilt.data.labels.push(timeLabel);
            chartTilt.data.labels.shift();
            chartTilt.data.datasets[0].data.push(tilt);
            chartTilt.data.datasets[0].data.shift();
            chartTilt.update();

            chartVib.data.labels.push(timeLabel);
            chartVib.data.labels.shift();
            chartVib.data.datasets[0].data.push(vibration);
            chartVib.data.datasets[0].data.shift();
            chartVib.update();
        },
        
        updateRSSI: function(nodeId, rssi) {
            const idxMap = { 'NODE_A': 0, 'NODE_B': 1, 'NODE_C': 2, 'NODE_D': 3 };
            const idx = idxMap[nodeId];
            if (idx !== undefined) {
                const timeLabel = new Date().toLocaleTimeString().split(' ')[0];
                chartRssi.data.labels.push(timeLabel);
                if (chartRssi.data.labels.length > 30) chartRssi.data.labels.shift();
                
                chartRssi.data.datasets[idx].data.push(rssi);
                if (chartRssi.data.datasets[idx].data.length > 30) {
                    chartRssi.data.datasets[idx].data.shift();
                }
                chartRssi.update();
            }
        },

        updatePrediction: function(actual, predicted) {
            const timeLabel = new Date().toLocaleTimeString().split(' ')[0];
            
            chartPred.data.labels.push(timeLabel);
            chartPred.data.labels.shift();
            
            chartPred.data.datasets[0].data.push(actual);
            chartPred.data.datasets[0].data.shift();
            
            chartPred.data.datasets[1].data.push(predicted);
            chartPred.data.datasets[1].data.shift();
            
            chartPred.update();
        }
    };
})();
