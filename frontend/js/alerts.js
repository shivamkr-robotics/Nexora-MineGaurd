// Alert System Logic
(function() {
    const alertList = document.getElementById('alert-list');
    const toastContainer = document.getElementById('toast-container');
    const btnAckAll = document.getElementById('btn-ack-all');

    const MAX_ALERTS = 15;

    btnAckAll.addEventListener('click', () => {
        alertList.innerHTML = '';
        toastContainer.innerHTML = '';
    });

    function playAlertSound(type) {
        if (type === 'critical') {
            // Short beep sound for demo
            const ctx = new (window.AudioContext || window.webkitAudioContext)();
            const osc = ctx.createOscillator();
            osc.type = 'square';
            osc.frequency.setValueAtTime(800, ctx.currentTime);
            osc.connect(ctx.destination);
            osc.start();
            osc.stop(ctx.currentTime + 0.2);
        }
    }

    window.alertsSystem = {
        addAlert: function(message, type = 'info', nodeId = 'SYS') {
            const time = new Date().toLocaleTimeString();
            
            // Add to sidebar
            const alertEl = document.createElement('div');
            alertEl.className = `alert-item ${type}`;
            alertEl.innerHTML = `
                <div>${message}</div>
                <div class="alert-meta">
                    <span>${nodeId}</span>
                    <span>${time}</span>
                </div>
            `;
            
            alertList.insertBefore(alertEl, alertList.firstChild);
            
            // Keep list bounded
            if (alertList.children.length > MAX_ALERTS) {
                alertList.removeChild(alertList.lastChild);
            }

            // Show toast
            const toast = document.createElement('div');
            toast.className = 'toast';
            if (type === 'critical') toast.style.borderLeftColor = '#ff1744';
            if (type === 'warning') toast.style.borderLeftColor = '#ffab00';
            toast.innerHTML = `<strong>${nodeId}</strong>: ${message}`;
            
            toastContainer.appendChild(toast);
            
            // Auto remove toast
            setTimeout(() => {
                if (toast.parentNode) {
                    toast.style.animation = 'slideIn 0.3s ease reverse forwards';
                    setTimeout(() => toast.remove(), 300);
                }
            }, 5000);

            if (type === 'critical') {
                playAlertSound('critical');
            }
        }
    };
})();
