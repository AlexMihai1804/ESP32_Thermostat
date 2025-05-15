// heatingControls.js
function loadHeatingMode() {
    fetch('/heating/mode')
        .then(response => response.json())
        .then(data => {
            const mode = data.mode;
            document.getElementById('heating-mode-display').innerText = `Mod Încălzire: ${mode}`;
            const heatingModeSelect = document.getElementById('heating-mode-select');
            heatingModeSelect.value = mode;

            if (mode === 'MANUAL') {
                showManualControls();
            } else {
                hideManualControls();
            }
        })
        .catch(error => {
            console.error('Error:', error);
            showErrorMessage('Nu s-a putut încărca modul de încălzire.');
        });
}

function setHeatingMode(mode) {
    fetch(`/heating/mode?mode=${encodeURIComponent(mode)}`, {
        method: 'PUT', headers: {'Content-Type': 'application/json'}
    })
        .then(response => {
            if (!response.ok) {
                return response.json().then(errData => {
                    throw new Error(errData.message || 'Eroare necunoscută');
                });
            }
            return response.json();
        })
        .then(data => {
            showSuccessMessage(data.message);
            loadHeatingMode();
            loadRelayStatus();
        })
        .catch(error => {
            showErrorMessage(`Eroare: ${error.message}`);
            console.error('Error:', error);
        });
}

function showManualControls() {
    const manualControls = document.getElementById('manual-controls');
    manualControls.classList.remove('hidden');
    loadManualMode();
}

function hideManualControls() {
    const manualControls = document.getElementById('manual-controls');
    manualControls.classList.add('hidden');
}

function loadManualMode() {
    fetch('/heating/manual')
        .then(response => response.json())
        .then(data => {
            const mode = data.mode;
            document.getElementById('manual-mode-display').innerText = `Mod Manual: ${mode}`;
            loadRelayStatus();
        })
        .catch(error => {
            console.error('Error:', error);
            showErrorMessage('Nu s-a putut încărca modul manual.');
        });
}

function setManualMode(mode) {
    fetch(`/heating/manual?mode=${encodeURIComponent(mode)}`, {
        method: 'PUT', headers: {'Content-Type': 'application/json'}
    })
        .then(response => {
            if (!response.ok) {
                return response.json().then(errData => {
                    throw new Error(errData.message || 'Eroare necunoscută');
                });
            }
            return response.json();
        })
        .then(data => {
            showSuccessMessage(data.message);
            loadManualMode();
        })
        .catch(error => {
            showErrorMessage(`Eroare: ${error.message}`);
            console.error('Error:', error);
        });
}

function loadRelayStatus() {
    fetch('/heating')
        .then(response => response.json())
        .then(data => {
            const isHeating = data.isHeating;
            document.getElementById('relay-status').innerText = `Stare Releu: ${isHeating ? 'Pornit' : 'Oprit'}`;
        })
        .catch(error => {
            console.error('Error:', error);
            showErrorMessage('Nu s-a putut încărca starea releului.');
        });
}

function setupManualControls() {
    const manualOnBtn = document.getElementById('set-manual-on');
    const manualOffBtn = document.getElementById('set-manual-off');
    if (manualOnBtn) {
        manualOnBtn.addEventListener('click', () => setManualMode('ON'));
    }
    if (manualOffBtn) {
        manualOffBtn.addEventListener('click', () => setManualMode('OFF'));
    }
    const heatingModeSelect = document.getElementById('heating-mode-select');
    heatingModeSelect.addEventListener('change', (event) => {
        const selectedMode = event.target.value;
        setHeatingMode(selectedMode);
    });
}
