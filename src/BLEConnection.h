#ifndef ESP32_TERMOSTAT_BLECONNECTION_H
#define ESP32_TERMOSTAT_BLECONNECTION_H

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

void initBLEConnection();

void beginAdvertisingReadings();

void readAdvertisingData(void * parameter);

// Add task handle for better task management
extern TaskHandle_t bleTaskHandle;

#endif //ESP32_TERMOSTAT_BLECONNECTION_H
