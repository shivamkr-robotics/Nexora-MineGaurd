// GIS Map Logic Using Leaflet
(function() {
    // Dhanbad, Jharkhand (Mine Area)
    const mapCenter = [23.7957, 86.4304];
    const map = L.map('gis-map').setView(mapCenter, 16);

    // CartoDB Dark Matter tiles
    L.tileLayer('https://cartodb-basemaps-{s}.global.ssl.fastly.net/dark_all/{z}/{x}/{y}{r}.png', {
        attribution: '&copy; <a href="https://www.openstreetmap.org/copyright">OpenStreetMap</a> &copy; <a href="https://carto.com/">CARTO</a>',
        subdomains: 'abcd',
        maxZoom: 20
    }).addTo(map);

    // Mine boundary polygon overlay
    const mineBoundary = [
        [23.7970, 86.4280],
        [23.7970, 86.4320],
        [23.7940, 86.4330],
        [23.7930, 86.4290]
    ];
    L.polygon(mineBoundary, {
        color: '#00e5ff',
        fillColor: '#00e5ff',
        fillOpacity: 0.1,
        weight: 1
    }).addTo(map);

    const nodeCoords = {
        'NODE_A': [23.7960, 86.4300],
        'NODE_B': [23.7955, 86.4308],
        'NODE_C': [23.7952, 86.4298],
        'NODE_D': [23.7958, 86.4312]
    };

    const markers = {};

    function getColor(status) {
        if (status === 'critical') return '#ff1744';
        if (status === 'warning') return '#ffab00';
        return '#00e676';
    }

    // Initialize markers
    Object.keys(nodeCoords).forEach(nodeId => {
        const marker = L.circleMarker(nodeCoords[nodeId], {
            radius: 8,
            fillColor: '#8888aa',
            color: '#fff',
            weight: 2,
            opacity: 1,
            fillOpacity: 0.8
        }).addTo(map);
        
        marker.bindPopup(`<b>${nodeId}</b><br>Waiting for data...`);
        markers[nodeId] = marker;
    });

    window.gisMap = {
        updateNodeStatus: function(nodeId, status, data) {
            if (!markers[nodeId]) return;
            
            const color = getColor(status);
            markers[nodeId].setStyle({ fillColor: color });

            const popupContent = `
                <div style="font-family:sans-serif;">
                    <h3 style="margin:0 0 5px 0; color:#333;">${nodeId}</h3>
                    <div style="color:#555; font-size:12px;">
                        <b>Tilt:</b> ${data.tilt ? data.tilt.toFixed(2) : '--'}°<br>
                        <b>Vibration:</b> ${data.vibration ? data.vibration.toFixed(2) : '--'} Hz<br>
                        <b>Gas CH4:</b> ${data.gas ? data.gas.toFixed(2) : '--'} %<br>
                        <b>Pressure:</b> ${data.pressure ? data.pressure.toFixed(1) : '--'} hPa<br>
                        <b>RSSI:</b> ${data.rssi || '--'} dBm<br>
                        <b>Battery:</b> ${data.battery || '--'} %<br>
                    </div>
                </div>
            `;
            markers[nodeId].setPopupContent(popupContent);
        }
    };
})();
