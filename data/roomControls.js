// roomControls.js

function loadRooms() {
    fetch('/rooms')
        .then(response => response.json())
        .then(data => {
            const roomsContainer = document.getElementById('rooms-container');
            roomsContainer.innerHTML = '';
            data.rooms.forEach(room => {
                const roomDiv = document.createElement('div');
                roomDiv.classList.add('room');
                const currentRoomMode = room.mode;
                let targetTemp = room.home_target_temperature;
                if (currentRoomMode === 'AWAY') {
                    targetTemp = room.away_target_temperature;
                } else if (currentRoomMode === 'NIGHT') {
                    targetTemp = room.night_target_temperature;
                }
                roomDiv.innerHTML = `
                    <h2>${room.room_name}</h2>
                    <p>Temperatură Curentă: ${room.current_temperature.toFixed(1)}°C</p>
                    <p>Umiditate Curentă: ${room.current_humidity.toFixed(1)}%</p>
                    <p>Temperatură Țintă: ${targetTemp.toFixed(1)}°C</p>
                    <p>Modul Camerei: ${currentRoomMode}</p>
                    <button class="room-settings-btn">Setări</button>
                `;
                roomDiv.querySelector('.room-settings-btn').addEventListener('click', () => openRoomSettingsPopup(room));
                roomsContainer.appendChild(roomDiv);
            });
        })
        .catch(error => {
            console.error('Error:', error);
            showErrorMessage('Nu s-au putut încărca camerele.');
        });
}

function openRoomSettingsPopup(roomData) {
    const popup = document.getElementById('room-settings-popup');
    const content = document.getElementById('popup-inner-content');
    content.innerHTML = '';
    content.innerHTML = `
        <h2>Setări pentru ${roomData.room_name}</h2>
        <label for="priority-${roomData.room_name}">Prioritate:</label>
        <input type="number" id="priority-${roomData.room_name}" value="${roomData.room_priority}" min="1" max="10">
        <div id="mode-buttons-${roomData.room_name}" class="mode-buttons">
            <h3>Modul Camerei</h3>
            <div class="buttons">
                <button id="set-home-mode-btn" class="btn ${roomData.mode === 'HOME' ? 'active' : ''}" data-mode="HOME">Acasă</button>
                <button id="set-away-mode-btn" class="btn ${roomData.mode === 'AWAY' ? 'active' : ''}" data-mode="AWAY">Plecat</button>
                <button id="set-night-mode-btn" class="btn ${roomData.mode === 'NIGHT' ? 'active' : ''}" data-mode="NIGHT">Noapte</button>
            </div>
        </div>
        <div id="mode-settings-${roomData.room_name}" class="mode-settings"></div>
        <div id="thermometers-${roomData.room_name}" class="thermometers-section">
            <h3>Termometre</h3>
            <ul id="thermometer-list-${roomData.room_name}">
                ${roomData.thermometers.map(thermo => `
                    <li>
                        MAC: ${thermo.mac}, Temp: ${thermo.temperature.toFixed(1)}°C, Umiditate: ${thermo.humidity.toFixed(1)}%
                        <button class="delete-thermometer-btn" data-mac="${thermo.mac}">Șterge</button>
                    </li>
                `).join('')}
            </ul>
            <input type="text" id="new-thermometer-mac-${roomData.room_name}" placeholder="MAC Termometru">
            <button id="add-thermometer-btn-${roomData.room_name}" class="btn btn-add">Adaugă Termometru</button>
        </div>

        <div class="buttons">
            <button id="update-room-settings-btn" class="btn btn-primary">Actualizează Setările</button>
            <button id="delete-room-btn" class="btn btn-secondary">Șterge Cameră</button>
        </div>
    `;
    const modeButtons = content.querySelectorAll('.mode-buttons .btn');
    modeButtons.forEach(btn => {
        btn.addEventListener('click', (e) => {
            modeButtons.forEach(b => b.classList.remove('active'));
            btn.classList.add('active');
            roomData.mode = btn.getAttribute('data-mode');
            showModeSettings(roomData);
        });
    });
    showModeSettings(roomData);
    document.getElementById('update-room-settings-btn').addEventListener('click', () => updateRoomSettings(roomData));
    document.getElementById('delete-room-btn').addEventListener('click', () => deleteRoom(roomData.room_name));
    document.getElementById(`add-thermometer-btn-${roomData.room_name}`).addEventListener('click', () => addThermometer(roomData.room_name));
    const deleteButtons = content.querySelectorAll('.delete-thermometer-btn');
    deleteButtons.forEach(btn => {
        const mac = btn.getAttribute('data-mac');
        btn.addEventListener('click', () => deleteThermometer(roomData.room_name, mac));
    });
    popup.classList.remove('hidden');
    popup.setAttribute('aria-hidden', 'false');
    const bodyClassList = document.body.classList;
    const popupContent = document.querySelector('.popup-content');
    if (bodyClassList.contains('dark-mode')) {
        popupContent.classList.add('dark-mode');
    } else {
        popupContent.classList.remove('dark-mode');
    }
}

function showModeSettings(roomData) {
    const modeSettingsDiv = document.getElementById(`mode-settings-${roomData.room_name}`);
    const selectedMode = roomData.mode;
    let temperatureValue = '';
    let lowOffsetValue = '';
    let highOffsetValue = '';
    if (selectedMode === 'HOME') {
        temperatureValue = roomData.home_target_temperature;
        lowOffsetValue = roomData.home_low_offset || 0;
        highOffsetValue = roomData.home_high_offset || 0;
    } else if (selectedMode === 'AWAY') {
        temperatureValue = roomData.away_target_temperature;
        lowOffsetValue = roomData.away_low_offset || 0;
        highOffsetValue = roomData.away_high_offset || 0;
    } else if (selectedMode === 'NIGHT') {
        temperatureValue = roomData.night_target_temperature;
        lowOffsetValue = roomData.night_low_offset || 0;
        highOffsetValue = roomData.night_high_offset || 0;
    }
    modeSettingsDiv.innerHTML = `
        <h3>Setări pentru modul "${selectedMode}"</h3>
        <label for="temp-${roomData.room_name}">Temperatură Țintă:</label>
        <input type="number" id="temp-${roomData.room_name}" value="${temperatureValue}" step="0.1">
        <label for="low-offset-${roomData.room_name}">Offset Temperatura Mică:</label>
        <input type="number" id="low-offset-${roomData.room_name}" value="${lowOffsetValue}" step="0.1">
        <label for="high-offset-${roomData.room_name}">Offset Temperatura Mare:</label>
        <input type="number" id="high-offset-${roomData.room_name}" value="${highOffsetValue}" step="0.1">
    `;
}

function updateRoomSettings(roomData) {
    const priorityInput = document.getElementById(`priority-${roomData.room_name}`);
    if (!validateNumberInput(priorityInput, 'Prioritatea camerei', 1, 10)) return;
    roomData.room_priority = parseFloat(priorityInput.value);
    const selectedMode = roomData.mode;
    const tempInput = document.getElementById(`temp-${roomData.room_name}`);
    const lowOffsetInput = document.getElementById(`low-offset-${roomData.room_name}`);
    const highOffsetInput = document.getElementById(`high-offset-${roomData.room_name}`);
    if (!validateNumberInput(tempInput, 'Temperatură Țintă')) return;
    if (!validateNumberInput(lowOffsetInput, 'Offset Temperatura Mică')) return;
    if (!validateNumberInput(highOffsetInput, 'Offset Temperatura Mare')) return;
    const temperatureValue = parseFloat(tempInput.value);
    const lowOffsetValue = parseFloat(lowOffsetInput.value);
    const highOffsetValue = parseFloat(highOffsetInput.value);
    if (selectedMode === 'HOME') {
        roomData.home_target_temperature = temperatureValue;
        roomData.home_low_offset = lowOffsetValue;
        roomData.home_high_offset = highOffsetValue;
    } else if (selectedMode === 'AWAY') {
        roomData.away_target_temperature = temperatureValue;
        roomData.away_low_offset = lowOffsetValue;
        roomData.away_high_offset = highOffsetValue;
    } else if (selectedMode === 'NIGHT') {
        roomData.night_target_temperature = temperatureValue;
        roomData.night_low_offset = lowOffsetValue;
        roomData.night_high_offset = highOffsetValue;
    }
    const updateButton = document.getElementById('update-room-settings-btn');
    updateButton.disabled = true;
    updateButton.textContent = 'Se actualizează...';
    fetch(`/rooms?room_name=${encodeURIComponent(roomData.room_name)}`, {
        method: 'PUT', headers: {'Content-Type': 'application/json'}, body: JSON.stringify(roomData)
    })
        .then(response => {
            updateButton.disabled = false;
            updateButton.textContent = 'Actualizează Setările';
            if (!response.ok) {
                return response.json().then(errData => {
                    throw new Error(errData.message || 'Eroare necunoscută');
                });
            }
            return response.json();
        })
        .then(data => {
            showSuccessMessage(data.message);
            closeRoomSettingsPopup();
            loadRooms();
        })
        .catch(error => {
            showErrorMessage(`Eroare: ${error.message}`);
            console.error('Error:', error);
        });
}

function deleteRoom(roomName) {
    Swal.fire({
        title: 'Ești sigur?',
        text: `Sigur doriți să ștergeți camera "${roomName}"?`,
        icon: 'warning',
        showCancelButton: true,
        confirmButtonText: 'Da, șterge',
        cancelButtonText: 'Anulează'
    }).then((result) => {
        if (result.isConfirmed) {
            fetch(`/rooms?room_name=${encodeURIComponent(roomName)}`, {
                method: 'DELETE', headers: {'Content-Type': 'application/json'}
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
                    closeRoomSettingsPopup();
                    loadRooms();
                })
                .catch(error => {
                    showErrorMessage(`Eroare: ${error.message}`);
                    console.error('Error:', error);
                });
        }
    });
}

function closeRoomSettingsPopup() {
    const popup = document.getElementById('room-settings-popup');
    popup.classList.add('hidden');
    popup.setAttribute('aria-hidden', 'true');
    const content = document.getElementById('popup-inner-content');
    content.innerHTML = '';
}

function addThermometer(roomName) {
    const macInput = document.getElementById(`new-thermometer-mac-${roomName}`);
    const macAddress = macInput.value.trim();
    if (!macAddress) {
        showErrorMessage('Vă rugăm să introduceți o adresă MAC.');
        macInput.focus();
        return;
    }
    const macRegex = /^([0-9A-Fa-f]{2}:){5}[0-9A-Fa-f]{2}$/;
    if (!macRegex.test(macAddress)) {
        showErrorMessage('Adresa MAC nu este validă. Formatul corect este XX:XX:XX:XX:XX:XX');
        macInput.focus();
        return;
    }
    const url = `/rooms/thermometers?room_name=${encodeURIComponent(roomName)}`;
    const payload = {
        mac: macAddress
    };
    const addButton = document.getElementById(`add-thermometer-btn-${roomName}`);
    addButton.disabled = true;
    addButton.textContent = 'Se adaugă...';
    fetch(url, {
        method: 'POST', headers: {'Content-Type': 'application/json'}, body: JSON.stringify(payload)
    })
        .then(response => {
            addButton.disabled = false;
            addButton.textContent = 'Adaugă Termometru';
            if (!response.ok) {
                return response.json().then(errData => {
                    throw new Error(errData.message || 'Eroare necunoscută');
                });
            }
            return response.json();
        })
        .then(data => {
            showSuccessMessage(data.message);
            macInput.value = '';
            fetch(`/rooms`)
                .then(response => response.json())
                .then(updatedData => {
                    const updatedRoom = updatedData.rooms.find(r => r.room_name === roomName);
                    if (updatedRoom) {
                        openRoomSettingsPopup(updatedRoom);
                    }
                });
        })
        .catch(error => {
            showErrorMessage(`Eroare: ${error.message}`);
            console.error('Error:', error);
        });
}

function deleteThermometer(roomName, macAddress) {
    Swal.fire({
        title: 'Ești sigur?',
        text: `Sigur doriți să ștergeți termometrul cu MAC "${macAddress}" din camera "${roomName}"?`,
        icon: 'warning',
        showCancelButton: true,
        confirmButtonText: 'Da, șterge',
        cancelButtonText: 'Anulează'
    }).then((result) => {
        if (result.isConfirmed) {
            const url = `/rooms/thermometers?room_name=${encodeURIComponent(roomName)}&mac=${encodeURIComponent(macAddress)}`;
            fetch(url, {
                method: 'DELETE', headers: {'Content-Type': 'application/json'}
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
                    fetch(`/rooms`)
                        .then(response => response.json())
                        .then(updatedData => {
                            const updatedRoom = updatedData.rooms.find(r => r.room_name === roomName);
                            if (updatedRoom) {
                                openRoomSettingsPopup(updatedRoom);
                            }
                        });
                })
                .catch(error => {
                    showErrorMessage(`Eroare: ${error.message}`);
                    console.error('Error:', error);
                });
        }
    });
}

function setupAddRoom() {
    const addRoomBtn = document.getElementById('add-room-btn');
    addRoomBtn.addEventListener('click', openAddRoomPopup);
}

function openAddRoomPopup() {
    const popup = document.getElementById('room-settings-popup');
    const content = document.getElementById('popup-inner-content');
    content.innerHTML = '';
    content.innerHTML = `
        <h2>Adaugă Cameră Nouă</h2>
        <label for="new-room-name">Nume Cameră:</label>
        <input type="text" id="new-room-name" placeholder="Introdu numele camerei" required>
        <button id="create-room-btn" class="btn btn-primary">Creează Cameră</button>
    `;
    document.getElementById('create-room-btn').addEventListener('click', createRoom);
    popup.classList.remove('hidden');
    popup.setAttribute('aria-hidden', 'false');
    const bodyClassList = document.body.classList;
    const popupContent = document.querySelector('.popup-content');
    if (bodyClassList.contains('dark-mode')) {
        popupContent.classList.add('dark-mode');
    } else {
        popupContent.classList.remove('dark-mode');
    }
}

function createRoom() {
    const roomNameInput = document.getElementById('new-room-name');
    const roomName = roomNameInput.value.trim();
    if (!roomName) {
        showErrorMessage('Vă rugăm să introduceți un nume de cameră.');
        roomNameInput.focus();
        return;
    }
    const invalidChars = /[^a-zA-Z0-9 ]/;
    if (invalidChars.test(roomName)) {
        showErrorMessage('Numele camerei conține caractere nepermise.');
        roomNameInput.focus();
        return;
    }
    roomExists(roomName).then(exists => {
        if (exists) {
            showErrorMessage('Camera deja există. Vă rugăm să alegeți un alt nume.');
            roomNameInput.focus();
        } else {
            const createButton = document.getElementById('create-room-btn');
            createButton.disabled = true;
            createButton.textContent = 'Se creează...';
            fetch('/rooms', {
                method: 'POST',
                headers: {'Content-Type': 'application/json'},
                body: JSON.stringify({room_name: roomName})
            })
                .then(response => {
                    createButton.disabled = false;
                    createButton.textContent = 'Creează Cameră';
                    if (!response.ok) {
                        return response.json().then(errData => {
                            throw new Error(errData.message || 'Eroare necunoscută');
                        });
                    }
                    return response.json();
                })
                .then(data => {
                    showSuccessMessage(data.message);
                    closeRoomSettingsPopup();
                    loadRooms();
                })
                .catch(error => {
                    showErrorMessage(`Eroare: ${error.message}`);
                    console.error('Error:', error);
                });
        }
    });
}

function roomExists(roomName) {
    return fetch('/rooms')
        .then(response => response.json())
        .then(data => {
            return data.rooms.some(room => room.room_name.toLowerCase() === roomName.toLowerCase());
        })
        .catch(error => {
            console.error('Error:', error);
            return false;
        });
}
