#include "HeatingControl.h"
#include <Arduino.h>
#include "globalSettings.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "HeatingHistory.h"

#define RELAY_PIN 26
time_t lastOn;
std::vector<RoomData> roomsData;

heatingStatus isHeatingNeeded() {
    if (heatingMode == MANUAL) {
        if (manualMode == ON_MANUAL) {
            return START;
        } else {
            return STOP;
        }
    } else if (heatingMode == OFF) {
        return STOP;
    }
    float heat = 0, actualHeat = 0;
    for (auto &room: rooms) {
        if (room.valid_thermometers()) {
            heat += room.get_room_priority();
            actualHeat += room.get_temperature_needs();
        }
    }
    if (heat == 0) {
        return STOP;
    }
    if (actualHeat < heat * -1) {
        return START;
    }
    if (actualHeat > heat) {
        return STOP;
    }
    return NORMAL;
}

void relay_init() {
    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, HIGH);
}

void updateRelayStatus() {
    heatingStatus status = isHeatingNeeded();
    if (status == START) {
        if (!isHeating) {
            isHeating = true;
            lastOn = time(nullptr);
            for (auto &room: rooms) {
                roomsData.push_back({room.getRoomTemperature(), room.getRoomTemperature(), room.get_humidity(),
                                     room.get_humidity(), room.get_room_priority(), room.get_room_name()});
            }
            digitalWrite(RELAY_PIN, LOW);
        }
    } else if (status == STOP) {
        if (isHeating) {
            isHeating = false;
            time_t now = time(nullptr);
            for (auto &room: rooms) {
                for (auto &roomData: roomsData) {
                    if (room.get_room_name() == roomData.name) {
                        roomData.endTemperature = room.getRoomTemperature();
                        roomData.endHumidity = room.get_humidity();
                    }
                }
            }
            RunTime run = {lastOn, now, roomsData};
            heatingHistory.addRunTime(run, true);
            digitalWrite(RELAY_PIN, HIGH);
        }
    }
}

void relaySyncTask(void *parameter) {
    while (true) {
        try {
            updateRelayStatus();
        } catch (const std::exception& e) {
            Serial.printf("Exception in relay task: %s\n", e.what());
        } catch (...) {
            Serial.println("Unknown error in relay task");
        }
        
        vTaskDelay(15 * 1000 / portTICK_PERIOD_MS);
    }
}

void start_relay_sync() {
    xTaskCreate(
            relaySyncTask,
            "RelayTask",
            3072,        // Increased stack size
            nullptr,
            2,           // Lower priority
            &relayTaskHandle);
            
    if (relayTaskHandle == NULL) {
        Serial.println("Failed to create relay task");
    }
}

void start_schedule_sync() {
    // Make sure scheduler mutex is initialized
    if (schedulerMutex == NULL) {
        schedulerMutex = xSemaphoreCreateMutex();
        if (schedulerMutex == NULL) {
            Serial.println("Failed to create scheduler mutex");
            return;
        }
    }
    
    // Increase stack size from 2048 to 4096 to prevent stack overflow
    xTaskCreate(
            update_schedule,
            "ScheduleTask",
            4096,
            nullptr,
            2,  // Lower priority
            &scheduleTaskHandle);
            
    if (scheduleTaskHandle == NULL) {
        Serial.println("Failed to create schedule task");
    }
}

void update_schedule(void *parameter) {
    while (true) {
        try {
            vTaskDelay(100 / portTICK_PERIOD_MS);
            
            // Check if mutex is initialized before attempting to take it
            if (schedulerMutex == NULL) {
                schedulerMutex = xSemaphoreCreateMutex();
                if (schedulerMutex == NULL) {
                    Serial.println("Failed to create scheduler mutex");
                    vTaskDelay(5000 / portTICK_PERIOD_MS);
                    continue;
                }
            }
            
            if (xSemaphoreTake(schedulerMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
                scheduler.updateSchedule();
                xSemaphoreGive(schedulerMutex);
            } else {
                Serial.println("Failed to take scheduler mutex");
            }
        } catch (const std::exception& e) {
            Serial.printf("Exception in schedule task: %s\n", e.what());
        } catch (...) {
            Serial.println("Unknown error in schedule task");
        }

        vTaskDelay(15 * 1000 / portTICK_PERIOD_MS);
    }
    
    vTaskDelete(NULL);
}

