// script.js

document.addEventListener('DOMContentLoaded', function () {
    loadRooms();
    setupThemeToggle();
    loadHeatingMode();
    loadRelayStatus();
    setupAddRoom();
    setupManualControls();
    setupScheduleControls();
    const closePopupBtn = document.getElementById('close-popup-btn');
    if (closePopupBtn) {
        closePopupBtn.addEventListener('click', closeRoomSettingsPopup);
    }
    const closeTimePopupBtn = document.getElementById('close-time-popup-btn');
    if (closeTimePopupBtn) {
        closeTimePopupBtn.addEventListener('click', closeTimeSettingsPopup);
    }
    const closeSchedulePopupBtn = document.getElementById('close-schedule-popup-btn');
    if (closeSchedulePopupBtn) {
        closeSchedulePopupBtn.addEventListener('click', closeScheduleViewPopup);
    }
    setInterval(() => {
        loadRooms();
        loadHeatingMode();
        loadRelayStatus();
    }, 30000);
});

function setupThemeToggle() {
    const toggleButton = document.getElementById('toggle-theme');
    const currentTheme = localStorage.getItem('theme') || 'light';
    if (currentTheme === 'dark') {
        document.body.classList.add('dark-mode');
        toggleButton.innerText = '☀️';
    } else {
        document.body.classList.remove('dark-mode');
        toggleButton.innerText = '🌙';
    }
    toggleButton.addEventListener('click', () => {
        document.body.classList.toggle('dark-mode');
        const isDarkMode = document.body.classList.contains('dark-mode');
        toggleButton.innerText = isDarkMode ? '☀️' : '🌙';
        localStorage.setItem('theme', isDarkMode ? 'dark' : 'light');
    });
}
