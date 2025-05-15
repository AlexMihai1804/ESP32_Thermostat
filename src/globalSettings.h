#ifndef ESP32_TERMOSTAT_GLOBALSETTINGS_H
#define ESP32_TERMOSTAT_GLOBALSETTINGS_H

#include <vector>
#include "Room.h"
#include "Scheduler.h"
#include "BLEAdvertisingReader.h"
#include "HeatingHistory.h"
#include "HeatingControl.h"
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>

extern std::vector<Room> rooms;
extern Scheduler scheduler;
extern BLEAdvertisingReader bleAdvertisingReader;
extern HeatingHistory heatingHistory;
extern bool isHeating;
extern heatingMode heatingMode;
extern manualMode manualMode;
// Add mutex for scheduler operations
extern portMUX_TYPE schedulerMux;
extern SemaphoreHandle_t schedulerMutex;
// Replace mutex with semaphore for BLE operations
extern SemaphoreHandle_t bleSemaphore;
// Add mutex for heating control operations
extern portMUX_TYPE heatingMux;

// Add task handles for better management
extern TaskHandle_t webServerTaskHandle;
extern TaskHandle_t bleTaskHandle;
extern TaskHandle_t scheduleTaskHandle;
extern TaskHandle_t relayTaskHandle;

// Add flag for global watchdog initialization
extern bool watchdogInitialized;

// Add function to initialize all semaphores
void initSemaphores();

#endif //ESP32_TERMOSTAT_GLOBALSETTINGS_H
