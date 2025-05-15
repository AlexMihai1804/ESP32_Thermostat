#ifndef ESP32_TERMOSTAT_SAVELOAD_H
#define ESP32_TERMOSTAT_SAVELOAD_H

void initSaveLoad();

void saveRooms();

void loadRooms();

void saveSchedule();

void loadSchedule();

void saveHistory();

void loadHistory();

void saveHeatingMode();

void loadHeatingMode();

#endif
