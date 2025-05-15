#include "LocalAPI.h"
#include "SaveLoad.h"
#include "globalSettings.h"
#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#include <vector>


AsyncWebServer server(80);

Room *findRoomByName(const std::string &room_name) {
    for (auto &room: rooms) {
        if (room.get_room_name() == room_name) {
            return &room;
        }
    }
    return nullptr;
}

void startWebServer(void *parameter) {
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(LittleFS, "/index.html", "text/html");
    });
    server.serveStatic("/", LittleFS, "/");
    server.on("/api/rooms", HTTP_GET, handleGetRooms);
    server.on("/api/rooms", HTTP_POST, [](AsyncWebServerRequest *request) {
    }, nullptr, handleCreateRoomBody);
    server.on("/api/rooms", HTTP_PUT, [](AsyncWebServerRequest *request) {
    }, nullptr, handleUpdateRoomBody);
    server.on("/api/rooms", HTTP_DELETE, handleDeleteRoom);
    server.on("/api/thermometers", HTTP_POST, [](AsyncWebServerRequest *request) {
    }, nullptr, handleAddThermometerBody);
    server.on("/api/thermometers", HTTP_DELETE, handleRemoveThermometer);
    server.on("/api/settings/reset", HTTP_GET, handleResetSettings);
    server.on("/api/heating/mode", HTTP_GET, handleGetHeatingMode);
    server.on("/api/heating/mode", HTTP_POST, handleSetHeatingMode);
    server.on("/api/heating/manual", HTTP_GET, handleGetManualMode);
    server.on("/api/heating/manual", HTTP_POST, handleSetManualMode);
    server.on("/api/heating/status", HTTP_GET, handleGetHeating);
    server.on("/api/schedule", HTTP_GET, handleGetSchedule);
    server.on("/api/schedule", HTTP_POST, [](AsyncWebServerRequest *request) {
    }, nullptr, handleSetScheduleBody);
    server.onNotFound([](AsyncWebServerRequest *request) {
        if (request->url().startsWith("/api/")) {
            request->send(404, "application/json", "{\"error\":\"Not found\"}");
            return;
        }
        request->send(LittleFS, request->url(), request->contentType());
    });
    server.begin();
}

std::string modeToString(themperature_modes mode) {
    switch (mode) {
        case HOME:
            return "HOME";
        case AWAY:
            return "AWAY";
        case NIGHT:
            return "NIGHT";
        case ANTIFREEZE:
            return "ANTIFREEZE";
        default:
            return "UNKNOWN";
    }
}

std::string heatingModeToString(enum heatingMode mode) {
    switch (mode) {
        case AUTO:
            return "AUTO";
        case MANUAL:
            return "MANUAL";
        case OFF:
            return "OFF";
        default:
            return "UNKNOWN";
    }
}

std::string manualModeToString(enum manualMode mode) {
    return (mode == ON_MANUAL) ? "ON" : "OFF";
}

void handleGetRooms(AsyncWebServerRequest *request) {
    JsonDocument doc;
    JsonArray roomsArray = doc["rooms"].to<JsonArray>();
    for (auto &room: rooms) {
        JsonObject roomObj = roomsArray.add<JsonObject>();
        roomObj["room_name"] = room.get_room_name();
        roomObj["current_temperature"] = room.getRoomTemperature();
        roomObj["current_humidity"] = room.get_humidity();
        roomObj["home_target_temperature"] = room.get_home_temperature();
        roomObj["home_low_offset"] = room.get_home_low_offset();
        roomObj["home_high_offset"] = room.get_home_high_offset();
        roomObj["away_target_temperature"] = room.get_away_temperature();
        roomObj["away_low_offset"] = room.get_away_low_offset();
        roomObj["away_high_offset"] = room.get_away_high_offset();
        roomObj["night_target_temperature"] = room.get_night_temperature();
        roomObj["night_low_offset"] = room.get_night_low_offset();
        roomObj["night_high_offset"] = room.get_night_high_offset();
        roomObj["room_priority"] = room.get_room_priority();
        roomObj["mode"] = modeToString(Room::get_room_mode());
        JsonArray thermos = roomObj["thermometers"].to<JsonArray>();
        for (int i = 0; i < room.get_thermometer_number(); i++) {
            JsonObject thermoObj = thermos.add<JsonObject>();
            thermoObj["mac"] = room.get_mac_by_index(i);
            thermoObj["temperature"] = room.get_temperature_by_index(i);
            thermoObj["humidity"] = room.get_humidity_by_index(i);
        }
    }
    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
}

void handleCreateRoomBody(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
    String body = String((char *) data, len);
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, body);
    if (error) {
        Serial.print("Eroare la parsarea JSON: ");
        Serial.println(error.c_str());
        request->send(400, "application/json", R"({"message":"JSON invalid"})");
        return;
    }
    if (!doc["room_name"].is<const char *>()) {
        Serial.println("room_name este obligatoriu");
        request->send(400, "application/json", R"({"message":"room_name este obligatoriu"})");
        return;
    }
    std::string room_name = doc["room_name"].as<const char *>();
    if (findRoomByName(room_name) != nullptr) {
        Serial.println("Camera deja există");
        request->send(400, "application/json", R"({"message":"Camera deja există"})");
        return;
    }
    float home_temp = doc["home_target_temperature"] | 22.0f;
    float home_low = doc["home_low_offset"] | 0.5f;
    float home_high = doc["home_high_offset"] | 0.5f;
    float priority = doc["room_priority"] | 5.0f;
    float away_temp = doc["away_target_temperature"] | 18.0f;
    float away_low = doc["away_low_offset"] | 0.75f;
    float away_high = doc["away_high_offset"] | 0.75f;
    float night_temp = doc["night_target_temperature"] | 21.0f;
    float night_low = doc["night_low_offset"] | 0.6f;
    float night_high = doc["night_high_offset"] | 0.6f;
    rooms.emplace_back(room_name, home_temp, home_low, home_high, priority, away_temp, away_low, away_high,
                       night_temp, night_low, night_high, true);
    saveRooms();
    JsonDocument responseDoc;
    responseDoc["message"] = "Camera a fost creată cu succes";
    responseDoc["room_name"] = room_name;
    String response;
    serializeJson(responseDoc, response);
    request->send(201, "application/json", response);
}

void handleUpdateRoomBody(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
    if (!request->hasParam("room_name")) {
        Serial.println("room_name nu este specificat");
        request->send(400, "application/json", R"({"message":"room_name nu este specificat"})");
        return;
    }
    String room_name = request->getParam("room_name")->value();
    Room *room = findRoomByName(room_name.c_str());
    if (room == nullptr) {
        Serial.println("Camera nu a fost găsită");
        request->send(404, "application/json", R"({"message":"Camera nu a fost găsită"})");
        return;
    }
    String body = String((char *) data, len);
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, body);
    if (error) {
        Serial.print("Eroare la parsarea JSON: ");
        Serial.println(error.c_str());
        request->send(400, "application/json", R"({"message":"JSON invalid"})");
        return;
    }
    if (doc["home_target_temperature"].is<float>()) {
        room->set_home_temperature(doc["home_target_temperature"].as<float>(), true);
    }
    if (doc["home_low_offset"].is<float>()) {
        room->set_home_low_offset(doc["home_low_offset"].as<float>(), true);
    }
    if (doc["home_high_offset"].is<float>()) {
        room->set_home_high_offset(doc["home_high_offset"].as<float>(), true);
    }
    if (doc["room_priority"].is<float>()) {
        room->set_room_priority(doc["room_priority"].as<float>(), true);
    }
    if (doc["away_target_temperature"].is<float>()) {
        room->set_away_temperature(doc["away_target_temperature"].as<float>(), true);
    }
    if (doc["away_low_offset"].is<float>()) {
        room->set_away_low_offset(doc["away_low_offset"].as<float>(), true);
    }
    if (doc["away_high_offset"].is<float>()) {
        room->set_away_high_offset(doc["away_high_offset"].as<float>(), true);
    }
    if (doc["night_target_temperature"].is<float>()) {
        room->set_night_temperature(doc["night_target_temperature"].as<float>(), true);
    }
    if (doc["night_low_offset"].is<float>()) {
        room->set_night_low_offset(doc["night_low_offset"].as<float>(), true);
    }
    if (doc["night_high_offset"].is<float>()) {
        room->set_night_high_offset(doc["night_high_offset"].as<float>(), true);
    }
    saveRooms();
    JsonDocument responseDoc;
    responseDoc["message"] = "Setările camerei au fost actualizate";
    String response;
    serializeJson(responseDoc, response);
    request->send(200, "application/json", response);
}

void handleDeleteRoom(AsyncWebServerRequest *request) {
    if (!request->hasParam("room_name")) {
        Serial.println("room_name nu este specificat");
        request->send(400, "application/json", R"({"message":"room_name nu este specificat"})");
        return;
    }
    String room_name = request->getParam("room_name")->value();
    auto it = std::find_if(rooms.begin(), rooms.end(), [&](const Room &room) -> bool {
        return room.get_room_name() == room_name.c_str();
    });
    if (it == rooms.end()) {
        Serial.println("Camera nu a fost găsită");
        request->send(404, "application/json", R"({"message":"Camera nu a fost găsită"})");
        return;
    }
    rooms.erase(it);
    saveRooms();
    JsonDocument responseDoc;
    responseDoc["message"] = "Camera " + room_name + " a fost ștearsă";
    String response;
    serializeJson(responseDoc, response);
    request->send(200, "application/json", response);
}

void handleAddThermometerBody(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
    if (!request->hasParam("room_name")) {
        request->send(400, "application/json", R"({"message":"room_name este obligatoriu"})");
        return;
    }
    String room_name = request->getParam("room_name")->value();
    Room *room = findRoomByName(room_name.c_str());
    if (room == nullptr) {
        request->send(404, "application/json", R"({"message":"Camera nu a fost găsită"})");
        return;
    }
    String body = String((char *) data, len);
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, body);
    if (error) {
        Serial.println(error.c_str());
        request->send(400, "application/json", R"({"message":"JSON invalid"})");
        return;
    }
    if (!doc["mac"].is<const char *>()) {
        request->send(400, "application/json", R"({"message":"mac este obligatoriu"})");
        return;
    }
    std::string mac = doc["mac"].as<const char *>();
    if (room->thermometerExist(mac)) {
        request->send(400, "application/json", R"({"message":"Termometrul deja există în cameră"})");
        return;
    }
    room->addThermometer(mac, false);
    JsonDocument responseDoc;
    responseDoc["message"] = "Termometrul a fost adăugat la camera " + room->get_room_name();
    String response;
    serializeJson(responseDoc, response);
    request->send(201, "application/json", response);
}

void handleRemoveThermometer(AsyncWebServerRequest *request) {
    if (!request->hasParam("room_name") || !request->hasParam("mac")) {
        request->send(400, "application/json", R"({"message":"room_name și mac sunt obligatorii"})");
        return;
    }
    String room_name = request->getParam("room_name")->value();
    String mac = request->getParam("mac")->value();
    Room *room = findRoomByName(room_name.c_str());
    if (room == nullptr) {
        request->send(404, "application/json", R"({"message":"Camera nu a fost găsită"})");
        return;
    }
    if (!room->thermometerExist(mac.c_str())) {
        request->send(404, "application/json", R"({"message":"Termometrul nu există în cameră"})");
        return;
    }
    room->removeThermometer(mac.c_str(), false);
    JsonDocument responseDoc;
    responseDoc["message"] = "Termometrul " + mac + " a fost eliminat din camera " + room_name;
    String response;
    serializeJson(responseDoc, response);
    request->send(200, "application/json", response);
}

void handleResetSettings(AsyncWebServerRequest *request) {
    rooms.clear();
    saveRooms();
    JsonDocument responseDoc;
    responseDoc["message"] = "Setările au fost resetate la valorile implicite";
    String response;
    serializeJson(responseDoc, response);
    request->send(200, "application/json", response);
}

void handleGetHeatingMode(AsyncWebServerRequest *request) {
    JsonDocument doc;
    doc["mode"] = heatingModeToString(heatingMode);
    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
}

void handleSetHeatingMode(AsyncWebServerRequest *request) {
    if (!request->hasParam("mode")) {
        Serial.println("mode nu este specificat");
        request->send(400, "application/json", R"({"message":"mode nu este specificat"})");
        return;
    }
    std::string mode = request->getParam("mode")->value().c_str();
    if (mode == "AUTO") {
        heatingMode = AUTO;
    } else if (mode == "MANUAL") {
        heatingMode = MANUAL;
    } else {
        heatingMode = OFF;
    }
    saveHeatingMode();
    JsonDocument doc;
    doc["message"] = "Modul de încălzire a fost setat la " + String(mode.c_str());
    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
}

void handleGetManualMode(AsyncWebServerRequest *request) {
    JsonDocument doc;
    doc["mode"] = manualModeToString(manualMode);
    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
}

void handleSetManualMode(AsyncWebServerRequest *request) {
    if (!request->hasParam("mode")) {
        Serial.println("mode nu este specificat");
        request->send(400, "application/json", R"({"message":"mode nu este specificat"})");
        return;
    }
    std::string mode = request->getParam("mode")->value().c_str();
    if (mode == "ON") {
        manualMode = ON_MANUAL;
    } else {
        manualMode = OFF_MANUAL;
    }
    saveHeatingMode();
    JsonDocument doc;
    doc["message"] = "Modul manual a fost setat la " + String(mode.c_str());
    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
}

void handleGetHeating(AsyncWebServerRequest *request) {
    JsonDocument doc;
    doc["isHeating"] = isHeating;
    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
}

void handleGetSchedule(AsyncWebServerRequest *request) {
    Serial.println("handleGetSchedule called");
    uint32_t start = millis();

    JsonDocument *doc = new JsonDocument();
    if (!doc) {
        request->send(500, "application/json", "{\"error\":\"Memory allocation failed\"}");
        return;
    }

    try {
        JsonArray scheduleArray = (*doc)["days"].to<JsonArray>();

        // Initialize mutex if it doesn't exist
        if (schedulerMutex == NULL) {
            Serial.println("Creating scheduler mutex in handleGetSchedule");
            schedulerMutex = xSemaphoreCreateMutex();
            if (schedulerMutex == NULL) {
                Serial.println("Failed to create scheduler mutex");
                request->send(500, "application/json", "{\"error\":\"Failed to create mutex\"}");
                delete doc;
                return;
            }
        }

        if (xSemaphoreTake(schedulerMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
            for (int i = 0; i < 7; i++) {
                JsonObject dayObj = scheduleArray.add<JsonObject>();
                JsonArray dayArray = dayObj["hours"].to<JsonArray>();
                for (int j = 0; j < 48; j++) {
                    switch (scheduler.getScheduleAtTime(i, j)) {
                        case HOME:
                            dayArray.add("HOME");
                            break;
                        case AWAY:
                            dayArray.add("AWAY");
                            break;
                        case NIGHT:
                            dayArray.add("NIGHT");
                            break;
                        default:
                            dayArray.add("ANTIFREEZE");
                            break;
                    }
                }
            }
            xSemaphoreGive(schedulerMutex);
        } else {
            Serial.println("Failed to take scheduler mutex in handleGetSchedule");
        }

        JsonArray userDirectivesArray = (*doc)["user_directives"].to<JsonArray>();
        for (auto &user_directive: scheduler.getUserDirectives()) {
            JsonObject directiveObj = userDirectivesArray.add<JsonObject>();
            directiveObj["start_time"] = user_directive.startTime;
            directiveObj["end_time"] = user_directive.finalTime;
            switch (user_directive.mode) {
                case HOME:
                    directiveObj["mode"] = "HOME";
                    break;
                case AWAY:
                    directiveObj["mode"] = "AWAY";
                    break;
                case NIGHT:
                    directiveObj["mode"] = "NIGHT";
                    break;
                default:
                    directiveObj["mode"] = "ANTIFREEZE";
                    break;
            }
        }

        JsonArray smartDirectivesArray = (*doc)["smart_directives"].to<JsonArray>();
        for (auto &smart_directive: scheduler.getSmartDirectives()) {
            JsonObject directiveObj = smartDirectivesArray.add<JsonObject>();
            directiveObj["start_time"] = smart_directive.startTime;
            directiveObj["end_time"] = smart_directive.finalTime;
            switch (smart_directive.mode) {
                case HOME:
                    directiveObj["mode"] = "HOME";
                    break;
                case AWAY:
                    directiveObj["mode"] = "AWAY";
                    break;
                case NIGHT:
                    directiveObj["mode"] = "NIGHT";
                    break;
                default:
                    directiveObj["mode"] = "ANTIFREEZE";
                    break;
            }
        }

        String response;
        serializeJson(*doc, response);
        request->send(200, "application/json", response);

        delete doc;
        Serial.println("handleGetSchedule completed in " + String(millis() - start) + "ms");
    } catch (const std::exception &e) {
        if (doc) delete doc;
        Serial.printf("Exception in handleGetSchedule: %s\n", e.what());
        request->send(500, "application/json", "{\"error\":\"Internal server error\"}");
    } catch (...) {
        if (doc) delete doc;
        Serial.println("Unknown error in handleGetSchedule");
        request->send(500, "application/json", "{\"error\":\"Internal server error\"}");
    }
}

void handleSetScheduleBody(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
    uint32_t start = millis();
    String body = String((char *) data, len);
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, body);
    if (error) {
        Serial.print("Eroare la parsarea JSON: ");
        Serial.println(error.c_str());
        request->send(400, "application/json", R"({"message":"JSON invalid"})");
        return;
    }
    if (!doc["day"].is<int>() || !doc["hour"].is<int>() || !doc["mode"].is<const char *>()) {
        request->send(400, "application/json", R"({"message":"day, hour și mode sunt obligatorii"})");
        return;
    }
    int day = doc["day"].as<int>();
    int hour = doc["hour"].as<int>();
    std::string mode = doc["mode"].as<const char *>();
    themperature_modes modeEnum;
    if (mode == "HOME") {
        modeEnum = HOME;
    } else if (mode == "AWAY") {
        modeEnum = AWAY;
    } else if (mode == "NIGHT") {
        modeEnum = NIGHT;
    } else {
        modeEnum = ANTIFREEZE;
    }

    // Initialize mutex if it doesn't exist
    if (schedulerMutex == NULL) {
        Serial.println("Creating scheduler mutex in handleSetScheduleBody");
        schedulerMutex = xSemaphoreCreateMutex();
        if (schedulerMutex == NULL) {
            Serial.println("Failed to create scheduler mutex");
            request->send(500, "application/json", "{\"error\":\"Failed to create mutex\"}");
            return;
        }
    }

    if (xSemaphoreTake(schedulerMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
        scheduler.setScheduleAtTime(day, hour, modeEnum);
        xSemaphoreGive(schedulerMutex);
    } else {
        Serial.println("Failed to take scheduler mutex in handleSetScheduleBody");
        request->send(503, "application/json", R"({"message":"Scheduler busy, try again"})");
        return;
    }

    saveSchedule();

    JsonDocument responseDoc;
    responseDoc["message"] = "Programul a fost actualizat";
    String response;
    serializeJson(responseDoc, response);
    request->send(200, "application/json", response);

    Serial.println("handleSetScheduleBody completed in " + String(millis() - start) + "ms");
}
