// LocalAPI.h

#ifndef ESP32_TERMOSTAT_LOCALAPI_H
#define ESP32_TERMOSTAT_LOCALAPI_H

#include <WiFiClientSecure.h>
#include <ESPAsyncWebServer.h>
#include "LittleFS.h"
#include <ArduinoJson.h>
#include "globalSettings.h"
#include "Room.h"
#include <vector>
#include <memory>
#include <string>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

extern AsyncWebServer server;
// Task handle already declared in globalSettings.h
// extern TaskHandle_t webServerTaskHandle;

void startWebServer(void *parameter);

// Function declarations for the request handlers
void handleGetRooms(AsyncWebServerRequest *request);
void handleCreateRoomBody(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total);
void handleUpdateRoomBody(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total);
void handleDeleteRoom(AsyncWebServerRequest *request);
void handleAddThermometerBody(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total);
void handleRemoveThermometer(AsyncWebServerRequest *request);
void handleResetSettings(AsyncWebServerRequest *request);
void handleGetHeatingMode(AsyncWebServerRequest *request);
void handleSetHeatingMode(AsyncWebServerRequest *request);
void handleGetManualMode(AsyncWebServerRequest *request);
void handleSetManualMode(AsyncWebServerRequest *request);
void handleGetHeating(AsyncWebServerRequest *request);
void handleGetSchedule(AsyncWebServerRequest *request);
void handleSetScheduleBody(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total);

Room *findRoomByName(const std::string &room_name);
std::string modeToString(themperature_modes mode);
themperature_modes stringToMode(const std::string &modeStr);

#endif // ESP32_TERMOSTAT_LOCALAPI_H
