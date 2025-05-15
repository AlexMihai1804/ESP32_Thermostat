// scheduleControls.js

let modifiedSlots = [];

function setupScheduleControls() {
    const getHomeNowBtn = document.getElementById('get-home-now-btn');
    const leaveNowBtn = document.getElementById('leave-now-btn');
    const getHomeAtTimeBtn = document.getElementById('get-home-at-time-btn');
    const leaveAtTimeBtn = document.getElementById('leave-at-time-btn');
    const editScheduleBtn = document.getElementById('edit-schedule-btn');
    const viewScheduleBtn = document.getElementById('view-schedule-btn');
    if (getHomeNowBtn) {
        getHomeNowBtn.addEventListener('click', getHomeNow);
    }
    if (leaveNowBtn) {
        leaveNowBtn.addEventListener('click', leaveNow);
    }
    if (getHomeAtTimeBtn) {
        getHomeAtTimeBtn.addEventListener('click', () => openTimeSettingsPopup('home'));
    }
    if (leaveAtTimeBtn) {
        leaveAtTimeBtn.addEventListener('click', () => openTimeSettingsPopup('away'));
    }
    if (editScheduleBtn) {
        editScheduleBtn.addEventListener('click', openEditSchedulePopup);
    }
    if (viewScheduleBtn) {
        viewScheduleBtn.addEventListener('click', openViewSchedulePopup);
    }
}

function getHomeNow() {
    fetch('/schedule/home_now', {
        method: 'POST', headers: {'Content-Type': 'application/json'}
    })
        .then(response => handleResponse(response))
        .then(data => {
            showSuccessMessage(data.message);
            loadHeatingMode();
        })
        .catch(error => {
            showErrorMessage(`Eroare: ${error.message}`);
            console.error('Error:', error);
        });
}

function leaveNow() {
    fetch('/schedule/leave_now', {
        method: 'POST', headers: {'Content-Type': 'application/json'}
    })
        .then(response => handleResponse(response))
        .then(data => {
            showSuccessMessage(data.message);
            loadHeatingMode();
        })
        .catch(error => {
            showErrorMessage(`Eroare: ${error.message}`);
            console.error('Error:', error);
        });
}

function handleResponse(response) {
    if (!response.ok) {
        return response.json().then(errData => {
            throw new Error(errData.message || 'Eroare necunoscută');
        });
    }
    return response.json();
}

function openTimeSettingsPopup(action) {
    const popup = document.getElementById('time-settings-popup');
    const content = document.getElementById('time-popup-inner-content');
    content.innerHTML = `
        <h2>${action === 'home' ? 'Setează Ora de Sosire Acasă' : 'Setează Ora de Plecare'}</h2>
        <label for="datetime-input">Data și Ora:</label>
        <input type="text" id="datetime-input" placeholder="Selectați data și ora" required>
        <button id="set-time-btn" class="btn btn-primary">Setează</button>
    `;
    const datetimeInput = document.getElementById('datetime-input');
    flatpickr(datetimeInput, {
        enableTime: true, dateFormat: "Y-m-d H:i", time_24hr: true, minuteIncrement: 30, minDate: "today"
    });
    document.getElementById('set-time-btn').addEventListener('click', () => {
        const selectedDate = datetimeInput.value;
        if (!selectedDate) {
            showErrorMessage('Vă rugăm să selectați data și ora.');
            return;
        }
        const date = new Date(selectedDate);
        if (isNaN(date.getTime())) {
            showErrorMessage('Data selectată nu este validă.');
            return;
        }
        const day = date.getDay();
        const hours = date.getHours();
        const minutes = date.getMinutes();
        const slotInDay = Math.floor(hours * 2 + minutes / 30);
        console.log(`Selected Date: ${selectedDate}`);
        console.log(`Day: ${day}, Slot in Day: ${slotInDay}`);
        if (action === 'home') {
            getHomeAtSlot(day, slotInDay);
        } else {
            leaveAtSlot(day, slotInDay);
        }
    });
    popup.classList.remove('hidden');
    popup.setAttribute('aria-hidden', 'false');
    applyCurrentThemeToPopup(popup);
}

function closeTimeSettingsPopup() {
    const popup = document.getElementById('time-settings-popup');
    popup.classList.add('hidden');
    popup.setAttribute('aria-hidden', 'true');
    const content = document.getElementById('time-popup-inner-content');
    content.innerHTML = '';
}

function getHomeAtSlot(day, slotInDay) {
    console.log(`Setting HOME mode for Day: ${day}, Slot: ${slotInDay}`);
    fetch('/schedule', {
        method: 'PUT', headers: {'Content-Type': 'application/json'}, body: JSON.stringify({
            day: day, hour: slotInDay, mode: "HOME"
        })
    })
        .then(response => handleResponse(response))
        .then(data => {
            showSuccessMessage(data.message);
            closeTimeSettingsPopup();
            loadHeatingMode();
        })
        .catch(error => {
            showErrorMessage(`Eroare: ${error.message}`);
            console.error('Error:', error);
        });
}

function leaveAtSlot(day, slotInDay) {
    console.log(`Setting AWAY mode for Day: ${day}, Slot: ${slotInDay}`);
    fetch('/schedule', {
        method: 'PUT', headers: {'Content-Type': 'application/json'}, body: JSON.stringify({
            day: day, hour: slotInDay, mode: "AWAY"
        })
    })
        .then(response => handleResponse(response))
        .then(data => {
            showSuccessMessage(data.message);
            closeTimeSettingsPopup();
            loadHeatingMode();
        })
        .catch(error => {
            showErrorMessage(`Eroare: ${error.message}`);
            console.error('Error:', error);
        });
}

function openViewSchedulePopup() {
    console.log('openViewSchedulePopup called');
    const popup = document.getElementById('schedule-view-popup');
    const content = document.getElementById('schedule-popup-inner-content');
    content.innerHTML = '<h2>Programul Săptămânal</h2>';
    fetch('/schedule')
        .then(response => {
            console.log('Received response from /schedule');
            if (!response.ok) {
                throw new Error('Nu s-a putut încărca programul.');
            }
            return response.json();
        })
        .then(data => {
            console.log('Schedule data:', data);
            const daysOfWeek = ['Duminică', 'Luni', 'Marți', 'Miercuri', 'Joi', 'Vineri', 'Sâmbătă'];
            const scheduleTable = document.createElement('table');
            scheduleTable.classList.add('schedule-table');
            const headerRow = document.createElement('tr');
            const timeHeader = document.createElement('th');
            timeHeader.innerText = 'Oră';
            headerRow.appendChild(timeHeader);
            daysOfWeek.forEach(day => {
                const dayHeader = document.createElement('th');
                dayHeader.innerText = day;
                headerRow.appendChild(dayHeader);
            });
            scheduleTable.appendChild(headerRow);
            for (let hour = 0; hour < 24; hour++) {
                const row = document.createElement('tr');
                const hourCell = document.createElement('td');
                hourCell.innerText = `${hour}:00 - ${hour}:59`;
                row.appendChild(hourCell);
                for (let day = 0; day < 7; day++) {
                    const cell = document.createElement('td');
                    const mode1 = data.days[day].hours[hour * 2];
                    const mode2 = data.days[day].hours[hour * 2 + 1];
                    if (mode1 === mode2) {
                        cell.innerText = mode1;
                    } else {
                        cell.innerText = `${mode1} / ${mode2}`;
                    }
                    row.appendChild(cell);
                }
                scheduleTable.appendChild(row);
            }
            content.appendChild(scheduleTable);
        })
        .catch(error => {
            console.error('Error:', error);
            showErrorMessage('Nu s-a putut încărca programul.');
        });
    popup.classList.remove('hidden');
    popup.setAttribute('aria-hidden', 'false');
    applyCurrentThemeToPopup(popup);
}

function closeScheduleViewPopup() {
    const popup = document.getElementById('schedule-view-popup');
    popup.classList.add('hidden');
    popup.setAttribute('aria-hidden', 'true');
    const content = document.getElementById('schedule-popup-inner-content');
    content.innerHTML = '';
}

function openEditSchedulePopup() {
    const popup = document.getElementById('schedule-edit-popup');
    const content = document.getElementById('schedule-edit-popup-inner-content');
    content.innerHTML = '<h2>Editează Programul Săptămânal</h2>';
    fetch('/schedule')
        .then(response => response.json())
        .then(data => {
            const daysOfWeek = ['Dum', 'Lun', 'Mar', 'Mie', 'Joi', 'Vin', 'Sâm'];
            const scheduleGrid = document.createElement('div');
            scheduleGrid.classList.add('schedule-grid');
            let isSelecting = false;
            let selectedSlots = [];
            scheduleGrid.addEventListener('touchstart', (e) => {
                isSelecting = true;
            });
            scheduleGrid.addEventListener('touchend', (e) => {
                isSelecting = false;
                selectedSlots = [];
            });
            for (let day = 0; day < 7; day++) {
                const dayColumn = document.createElement('div');
                dayColumn.classList.add('day-column');
                const dayHeader = document.createElement('div');
                dayHeader.classList.add('day-header');
                dayHeader.innerText = daysOfWeek[day];
                dayColumn.appendChild(dayHeader);
                for (let slot = 0; slot < 48; slot++) {
                    const timeSlot = document.createElement('div');
                    timeSlot.classList.add('time-slot');
                    const hour = Math.floor(slot / 2);
                    const minute = slot % 2 === 0 ? '00' : '30';
                    timeSlot.dataset.time = `${hour}:${minute}`;
                    timeSlot.dataset.day = day;
                    timeSlot.dataset.hour = slot;
                    const mode = data.days[day].hours[slot];
                    timeSlot.dataset.mode = mode;
                    updateTimeSlotAppearance(timeSlot, mode);
                    timeSlot.addEventListener('mousedown', (e) => {
                        e.preventDefault();
                        isSelecting = true;
                        toggleTimeSlotSelection(timeSlot, selectedSlots);
                    });
                    timeSlot.addEventListener('touchstart', (e) => {
                        e.preventDefault();
                        isSelecting = true;
                        toggleTimeSlotSelection(timeSlot, selectedSlots);
                    });
                    timeSlot.addEventListener('mouseenter', () => {
                        if (isSelecting) {
                            toggleTimeSlotSelection(timeSlot, selectedSlots);
                        }
                    });
                    timeSlot.addEventListener('touchmove', (e) => {
                        const touch = e.touches[0];
                        const target = document.elementFromPoint(touch.clientX, touch.clientY);
                        if (target && target.classList.contains('time-slot') && !selectedSlots.includes(target)) {
                            toggleTimeSlotSelection(target, selectedSlots);
                        }
                    });
                    timeSlot.addEventListener('mouseup', () => {
                        isSelecting = false;
                    });
                    timeSlot.addEventListener('touchend', () => {
                        isSelecting = false;
                    });
                    dayColumn.appendChild(timeSlot);
                }
                scheduleGrid.appendChild(dayColumn);
            }
            document.addEventListener('mouseup', () => {
                isSelecting = false;
            });
            document.addEventListener('touchend', () => {
                isSelecting = false;
            });
            content.appendChild(scheduleGrid);
            const modeSelect = document.createElement('select');
            modeSelect.innerHTML = `
                <option value="HOME">Acasă</option>
                <option value="AWAY">Plecat</option>
                <option value="NIGHT">Noapte</option>
                <option value="ANTIFREEZE">Antigel</option>
                <option value="OFF">Oprit</option>
            `;
            modeSelect.classList.add('mode-select');
            const applyButton = document.createElement('button');
            applyButton.classList.add('btn', 'btn-primary');
            applyButton.innerText = 'Aplică Mod';
            applyButton.addEventListener('click', () => {
                const selectedMode = modeSelect.value;
                console.log(`Selected Mode to Apply: ${selectedMode}`); // Debugging
                applyModeToSelectedSlots(selectedSlots, selectedMode);
            });
            const saveButton = document.createElement('button');
            saveButton.classList.add('btn', 'btn-primary');
            saveButton.innerText = 'Salvează Programul';
            saveButton.addEventListener('click', () => saveSchedule());
            const buttonContainer = document.createElement('div');
            buttonContainer.classList.add('buttons');
            buttonContainer.appendChild(modeSelect);
            buttonContainer.appendChild(applyButton);
            buttonContainer.appendChild(saveButton);
            content.appendChild(buttonContainer);
        })
        .catch(error => {
            console.error('Error:', error);
            showErrorMessage('Nu s-a putut încărca programul pentru editare.');
        });
    popup.classList.remove('hidden');
    popup.setAttribute('aria-hidden', 'false');
    applyCurrentThemeToPopup(popup);
}

function closeEditSchedulePopup() {
    const popup = document.getElementById('schedule-edit-popup');
    popup.classList.add('hidden');
    popup.setAttribute('aria-hidden', 'true');
    const content = document.getElementById('schedule-edit-popup-inner-content');
    content.innerHTML = '';
}

function updateTimeSlotAppearance(timeSlot, mode) {
    timeSlot.className = 'time-slot';
    timeSlot.classList.add(`mode-${mode}`);
    timeSlot.innerText = mode;
}

function toggleTimeSlotSelection(timeSlot, selectedSlots) {
    if (timeSlot.classList.contains('selected')) {
        timeSlot.classList.remove('selected');
        const index = selectedSlots.indexOf(timeSlot);
        if (index > -1) {
            selectedSlots.splice(index, 1);
        }
    } else {
        timeSlot.classList.add('selected');
        selectedSlots.push(timeSlot);
        markSlotAsModified(timeSlot);
    }
}

function markSlotAsModified(timeSlot) {
    const day = parseInt(timeSlot.dataset.day);
    const hour = parseInt(timeSlot.dataset.hour);
    const mode = timeSlot.dataset.mode;
    console.log(`Marking as modified - Day: ${day}, Hour: ${hour}, Mode: ${mode}`);
    const exists = modifiedSlots.some(d => d.day === day && d.hour === hour);
    if (!exists) {
        modifiedSlots.push({day, hour, mode});
        timeSlot.classList.add('modified');
    } else {
        const slotIndex = modifiedSlots.findIndex(d => d.day === day && d.hour === hour);
        if (slotIndex !== -1) {
            modifiedSlots[slotIndex].mode = mode;
            console.log(`Updated slot mode - Day: ${day}, Hour: ${hour}, Mode: ${mode}`);
        }
    }
}

function applyModeToSelectedSlots(selectedSlots, mode) {
    console.log(`Applying mode: ${mode} to ${selectedSlots.length} slots`);
    selectedSlots.forEach(slot => {
        slot.dataset.mode = mode;
        updateTimeSlotAppearance(slot, mode);
        slot.classList.remove('selected');
        markSlotAsModified(slot);
    });
    selectedSlots.length = 0;
}

function saveSchedule() {
    console.log('Saving schedule...');
    if (modifiedSlots.length === 0) {
        showErrorMessage('Nu există modificări de salvat.');
        console.warn('No modifications found in modifiedSlots.');
        return;
    }
    const scheduleData = modifiedSlots.map(slot => {
        const {day, hour, mode} = slot;
        if (day < 0 || day > 6 || hour < 0 || hour > 47) {
            console.warn(`Slot invalid: day=${day}, hour=${hour}`);
            return null; // Ignorăm sloturile invalide
        }
        return {day, hour, mode};
    }).filter(slot => slot !== null);
    console.log('Schedule Data to Send:', scheduleData);
    Promise.all(scheduleData.map(directive => {
        console.log('Sending directive:', directive);
        return fetch('/schedule', {
            method: 'PUT', headers: {'Content-Type': 'application/json'}, body: JSON.stringify({
                day: directive.day, hour: directive.hour, mode: directive.mode
            })
        })
            .then(response => handleResponse(response));
    }))
        .then(results => {
            console.log('All directives saved successfully:', results);
            showSuccessMessage('Programul a fost actualizat cu succes');
            closeEditSchedulePopup();
            loadHeatingMode();
            resetModifiedSlots();
        })
        .catch(error => {
            console.error('Error saving schedule:', error);
            showErrorMessage(`Eroare: ${error.message}`);
        });
}

function resetModifiedSlots() {
    console.log('Resetting modifiedSlots...');
    modifiedSlots = [];
    const modifiedElements = document.querySelectorAll('.time-slot.modified');
    modifiedElements.forEach(slot => slot.classList.remove('modified'));
}

function applyCurrentThemeToPopup(popup) {
    const bodyClassList = document.body.classList;
    const popupContent = popup.querySelector('.popup-content');
    if (bodyClassList.contains('dark-mode')) {
        popupContent.classList.add('dark-mode');
    } else {
        popupContent.classList.remove('dark-mode');
    }
}

document.addEventListener('DOMContentLoaded', function () {
    setupScheduleControls();
    const closeViewPopupBtn = document.getElementById('close-schedule-popup-btn');
    if (closeViewPopupBtn) {
        closeViewPopupBtn.addEventListener('click', closeScheduleViewPopup);
    }
    const closeEditPopupBtn = document.getElementById('close-schedule-edit-popup-btn');
    if (closeEditPopupBtn) {
        closeEditPopupBtn.addEventListener('click', closeEditSchedulePopup);
    }
    const closeTimePopupBtn = document.getElementById('close-time-popup-btn');
    if (closeTimePopupBtn) {
        closeTimePopupBtn.addEventListener('click', closeTimeSettingsPopup);
    }
});
