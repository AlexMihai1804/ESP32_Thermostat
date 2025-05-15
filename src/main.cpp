#include <Arduino.h>
#include "WiFiConnection.h"
#include "SaveLoad.h"
#include "BLEConnection.h"
#include "Room.h"
#include "globalSettings.h"
#include "HeatingControl.h"
#include "LocalAPI.h"
#include "OTAUpdate.h"

void print_free_mem() {
    Serial.printf("Free heap: %u bytes\n", ESP.getFreeHeap());
}

void WebServerTask(void *pv);
constexpr uint32_t WEB_TASK_STACK =  32* 1024;   // 12kB stack

void setup() {
    Serial.begin(115200);

    initSaveLoad();
    initBLEConnection();
    initWiFi(false);
    setupOTAUpdate();

    relay_init();

    loadRooms();
    loadSchedule();
    loadHistory();
    loadHeatingMode();
    Serial.println("Starting advertising readings");
    beginAdvertisingReadings();
    start_schedule_sync();
    Serial.println("Starting web server task");
    vTaskDelay(10000 / portTICK_PERIOD_MS);
    xTaskCreatePinnedToCore(
        WebServerTask,
        "WebSrv",
        WEB_TASK_STACK,
        nullptr,
        1,
        nullptr,
        0);
    start_relay_sync();
    startOTAUpdate();

    Serial.println("Setup complete – tasks launched");
}

// ────────────────────────────────────────────────────────────────
//                               loop()
// ────────────────────────────────────────────────────────────────
void loop() {
    vTaskDelay(portMAX_DELAY);
}


void WebServerTask(void *pv) {
    Serial.println("Starting Web server task");
    startWebServer(nullptr);
    for (;;) {
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}
