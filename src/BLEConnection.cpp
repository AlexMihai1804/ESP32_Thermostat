#include "BLEConnection.h"
#include "globalSettings.h"
#include <NimBLEDevice.h>
#include <freertos/semphr.h>
#include <Arduino.h>

extern SemaphoreHandle_t bleSemaphore;

void readAdvertisingData(void *parameter);

void initBLEConnection() {
    NimBLEDevice::init("Thermostat");
    
    // Ensure semaphore is created properly
    if (bleSemaphore == NULL) {
        bleSemaphore = xSemaphoreCreateMutex();
        if (bleSemaphore == NULL) {
            Serial.println("Failed to create BLE semaphore in initBLEConnection");
        }
    }
}

void beginAdvertisingReadings() {
    if (bleSemaphore == nullptr) {
        bleSemaphore = xSemaphoreCreateMutex();
        if (!bleSemaphore) {
            Serial.println("Failed to create BLE semaphore");
            return;
        }
    }
    xTaskCreatePinnedToCore(readAdvertisingData, "readAdvertisingData", 12288, nullptr, 3, &bleTaskHandle, 1);
}

void readAdvertisingData(void *parameter) {
    for (;;) {
        if (xSemaphoreTake(bleSemaphore, pdMS_TO_TICKS(500)) == pdTRUE) {
            bleAdvertisingReader.initAllThermometers();
            xSemaphoreGive(bleSemaphore);
        }
        vTaskDelay(50 / portTICK_PERIOD_MS);

        for (int i = 0; i < 4; ++i) {
            if (xSemaphoreTake(bleSemaphore, pdMS_TO_TICKS(500)) == pdTRUE) {
                bleAdvertisingReader.readAdvertising(0.5);
                xSemaphoreGive(bleSemaphore);
            }
            vTaskDelay(50 / portTICK_PERIOD_MS);
        }

        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
