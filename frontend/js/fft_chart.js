// FFT Bar Chart Logic
(function() {
    const ctx = document.getElementById('chart-fft').getContext('2d');
    
    // Initial empty bins
    const labels = Array.from({length: 20}, (_, i) => i * 5); // 0 to ~100Hz
    const data = Array(20).fill(0);

    const gradient = ctx.createLinearGradient(0, 0, 0, 400);
    gradient.addColorStop(0, '#00e5ff');
    gradient.addColorStop(1, 'rgba(0, 229, 255, 0.1)');

    const chart = new Chart(ctx, {
        type: 'bar',
        data: {
            labels: labels,
            datasets: [{
                label: 'Amplitude',
                data: data,
                backgroundColor: gradient,
                borderColor: '#00e5ff',
                borderWidth: 1
            }]
        },
        options: {
            responsive: true,
            maintainAspectRatio: false,
            animation: false, // Performance
            scales: {
                y: {
                    beginAtZero: true,
                    grid: { color: '#2a2a3a' },
                    ticks: { color: '#8888aa' }
                },
                x: {
                    grid: { display: false },
                    ticks: { color: '#8888aa', maxTicksLimit: 10 },
                    title: { display: true, text: 'Frequency (Hz)', color: '#8888aa' }
                }
            },
            plugins: {
                legend: { display: false }
            }
        }
    });

    window.fftChart = {
        updateFFT: function(freqBins, ampBins, dominantFreq) {
            chart.data.labels = freqBins;
            chart.data.datasets[0].data = ampBins;
            
            // Highlight dominant frequency
            const bgColors = freqBins.map(f => {
                // If it's near dominant, highlight orange
                if (dominantFreq && Math.abs(f - dominantFreq) < 2.5) {
                    return '#ffab00';
                }
                return gradient;
            });
            chart.data.datasets[0].backgroundColor = bgColors;
            
            chart.update();
        }
    };
})();
