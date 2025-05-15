#include "SaveLoad.h"
#include <LittleFS.h>
#include <ArduinoJson.h>
#include "globalSettings.h"

void initSaveLoad() {
    if (!LittleFS.begin(true)) {
        Serial.println("An Error has occurred while mounting LittleFS");
        return;
    }
    Serial.println("LittleFS mounted successfully");
}

void saveRooms() {
    if (!LittleFS.exists("/rooms.json")) {
        Serial.println("Rooms file does not exist. Creating new file.");
        LittleFS.open("/rooms.json", "w").close();
    }
    File file = LittleFS.open("/rooms.json", "w");
    if (!file) {
        Serial.println("There was an error opening the file for writing");
        return;
    }
    JsonDocument doc;
    JsonArray roomsArray = doc["rooms"].to<JsonArray>();
    if (rooms.empty()) {
        Serial.println("No rooms to save");
        return;
    }
    for (Room room: rooms) {
        JsonObject roomObject = roomsArray.add<JsonObject>();
        roomObject["name"] = room.get_room_name();
        roomObject["home_temperature"] = room.get_home_temperature();
        roomObject["home_low_offset"] = room.get_home_low_offset();
        roomObject["home_high_offset"] = room.get_home_high_offset();
        roomObject["night_temperature"] = room.get_night_temperature();
        roomObject["night_low_offset"] = room.get_night_low_offset();
        roomObject["night_high_offset"] = room.get_night_high_offset();
        roomObject["away_temperature"] = room.get_away_temperature();
        roomObject["away_low_offset"] = room.get_away_low_offset();
        roomObject["away_high_offset"] = room.get_away_high_offset();
        roomObject["priority"] = room.get_room_priority();
        JsonArray thermometersArray = roomObject["thermometers"].to<JsonArray>();
        for (int i = 0; i < room.get_thermometer_number(); i++) {
            JsonObject thermometerObject = thermometersArray.add<JsonObject>();
            thermometerObject["mac"] = room.get_mac_by_index(i);
        }
    }
    doc["time"] = time(nullptr);
    if (serializeJson(doc, file) == 0) {
        Serial.println("Failed to write to file");
    }
    file.close();
}

void loadRooms() {
    if (!LittleFS.exists("/rooms.json")) {
        Serial.println("Rooms file does not exist. Creating new file.");
        saveRooms();
    }
    File file = LittleFS.open("/rooms.json", "r");
    if (!file) {
        Serial.println("There was an error opening the file for reading");
        return;
    }
    rooms.clear();
    JsonDocument doc;
    Serial.println("Loading rooms:");
    DeserializationError error = deserializeJson(doc, file);
    if (error) {
        Serial.print("Deserialization failed: ");
        Serial.println(error.f_str());
        return;
    }
    JsonArray roomsArray = doc["rooms"].as<JsonArray>();
    for (JsonObject roomObject: roomsArray) {
        Room room(roomObject["name"].as<std::string>(), roomObject["home_temperature"].as<float>(),
                  roomObject["home_low_offset"].as<float>(), roomObject["home_high_offset"].as<float>(),
                  roomObject["priority"].as<float>(), roomObject["away_temperature"].as<float>(),
                  roomObject["away_low_offset"].as<float>(), roomObject["away_high_offset"].as<float>(),
                  roomObject["night_temperature"].as<float>(), roomObject["night_low_offset"].as<float>(),
                  roomObject["night_high_offset"].as<float>(), true);
        JsonArray thermometersArray = roomObject["thermometers"].as<JsonArray>();
        for (JsonObject thermometerObject: thermometersArray) {
            room.addThermometer(thermometerObject["mac"].as<std::string>(), true);
        }
        rooms.push_back(room);
    }
    file.close();
}

void saveSchedule() {
    if (!LittleFS.exists("/schedule.json")) {
        Serial.println("Schedule file does not exist. Creating new file.");
        LittleFS.open("/schedule.json", "w").close();
    }
    File file = LittleFS.open("/schedule.json", "w");
    if (!file) {
        Serial.println("There was an error opening the file for writing");
        return;
    }
    JsonDocument doc;
    JsonArray daysArray = doc["days"].to<JsonArray>();
    for (int i = 0; i < 7; i++) {
        JsonObject dayObject = daysArray.add<JsonObject>();
        JsonArray hoursArray = dayObject["hours"].to<JsonArray>();
        for (int j = 0; j < 48; j++) {
            switch (scheduler.getScheduleAtTime(i, j)) {
                case HOME:
                    hoursArray.add("HOME");
                    break;
                case NIGHT:
                    hoursArray.add("NIGHT");
                    break;
                case AWAY:
                    hoursArray.add("AWAY");
                    break;
                case ANTIFREEZE:
                    hoursArray.add("ANTIFREEZE");
                    break;
                default:
                    hoursArray.add("HOME");
                    break;
            }
        }
    }
    JsonArray userDirectivesArray = doc["user_directives"].to<JsonArray>();
    for (directive userDirective: scheduler.getUserDirectives()) {
        JsonObject userDirectiveObject = userDirectivesArray.add<JsonObject>();
        userDirectiveObject["start_time"] = userDirective.startTime;
        userDirectiveObject["end_time"] = userDirective.finalTime;
        switch (userDirective.mode) {
            case HOME:
                userDirectiveObject["mode"] = "HOME";
                break;
            case NIGHT:
                userDirectiveObject["mode"] = "NIGHT";
                break;
            case AWAY:
                userDirectiveObject["mode"] = "AWAY";
                break;
            case ANTIFREEZE:
                userDirectiveObject["mode"] = "ANTIFREEZE";
                break;
            default:
                userDirectiveObject["mode"] = "HOME";
                break;
        }
    }
    JsonArray smartDirectivesArray = doc["smart_directives"].to<JsonArray>();
    for (directive smartDirective: scheduler.getSmartDirectives()) {
        JsonObject smartDirectiveObject = smartDirectivesArray.add<JsonObject>();
        smartDirectiveObject["start_time"] = smartDirective.startTime;
        smartDirectiveObject["end_time"] = smartDirective.finalTime;
        switch (smartDirective.mode) {
            case HOME:
                smartDirectiveObject["mode"] = "HOME";
                break;
            case NIGHT:
                smartDirectiveObject["mode"] = "NIGHT";
                break;
            case AWAY:
                smartDirectiveObject["mode"] = "AWAY";
                break;
            case ANTIFREEZE:
                smartDirectiveObject["mode"] = "ANTIFREEZE";
                break;
            default:
                smartDirectiveObject["mode"] = "HOME";
                break;
        }
    }
    if (serializeJson(doc, file) == 0) {
        Serial.println("Failed to write to file");
    } else {
        Serial.println("Schedule saved successfully.");
    }
    file.close();
}

void loadSchedule() {
    if (!LittleFS.exists("/schedule.json")) {
        Serial.println("Schedule file does not exist. Creating new file.");
        saveSchedule();
    }
    File file = LittleFS.open("/schedule.json", "r");
    if (!file) {
        Serial.println("There was an error opening the file for reading");
        return;
    }
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, file);
    if (error) {
        Serial.println("Failed to read file, using default configuration");
        return;
    }
    JsonArray daysArray = doc["days"].as<JsonArray>();
    for (int i = 0; i < 7; i++) {
        JsonObject dayObject = daysArray[i].as<JsonObject>();
        JsonArray hoursArray = dayObject["hours"].as<JsonArray>();
        for (int j = 0; j < 48; j++) {
            if (hoursArray[j] == "HOME") {
                scheduler.setScheduleAtTime(i, j, HOME, true);
            } else if (hoursArray[j] == "NIGHT") {
                scheduler.setScheduleAtTime(i, j, NIGHT, true);
            } else if (hoursArray[j] == "AWAY") {
                scheduler.setScheduleAtTime(i, j, AWAY, true);
            } else if (hoursArray[j] == "ANTIFREEZE") {
                scheduler.setScheduleAtTime(i, j, ANTIFREEZE, true);
            } else if (hoursArray[j] == "HOME") {
                scheduler.setScheduleAtTime(i, j, HOME, true);
            } else {
                scheduler.setScheduleAtTime(i, j, HOME, true);
            }
        }
    }
    JsonArray userDirectivesArray = doc["user_directives"].as<JsonArray>();
    for (JsonObject userDirectiveObject: userDirectivesArray) {
        time_t start_time = userDirectiveObject["start_time"].as<int>();
        time_t end_time = userDirectiveObject["end_time"].as<int>();
        themperature_modes mode;
        if (userDirectiveObject["mode"] == "HOME") {
            mode = HOME;
        } else if (userDirectiveObject["mode"] == "NIGHT") {
            mode = NIGHT;
        } else if (userDirectiveObject["mode"] == "AWAY") {
            mode = AWAY;
        } else if (userDirectiveObject["mode"] == "ANTIFREEZE") {
            mode = ANTIFREEZE;
        } else {
            mode = HOME;
        }
        scheduler.addUserDirective(start_time, end_time, mode, true);
    }
    JsonArray smartDirectivesArray = doc["smart_directives"].as<JsonArray>();
    for (JsonObject smartDirectiveObject: smartDirectivesArray) {
        time_t start_time = smartDirectiveObject["start_time"].as<int>();
        time_t end_time = smartDirectiveObject["end_time"].as<int>();
        themperature_modes mode;
        if (smartDirectiveObject["mode"] == "HOME") {
            mode = HOME;
        } else if (smartDirectiveObject["mode"] == "NIGHT") {
            mode = NIGHT;
        } else if (smartDirectiveObject["mode"] == "AWAY") {
            mode = AWAY;
        } else if (smartDirectiveObject["mode"] == "ANTIFREEZE") {
            mode = ANTIFREEZE;
        } else {
            mode = HOME;
        }
        scheduler.addSmartDirective(start_time, end_time, mode, true);
    }
    file.close();
}

void saveHistory() {
    if (!LittleFS.exists("/history.json")) {
        Serial.println("History file does not exist. Creating new file.");
        File file = LittleFS.open("/history.json", "w");
        if (!file) {
            Serial.println("Failed to create history file.");
            return;
        }
        file.close();
    }
    File file = LittleFS.open("/history.json", "w");
    if (!file) {
        Serial.println("There was an error opening the file for writing");
        return;
    }
    JsonDocument doc;
    JsonArray daysArray = doc["days"].to<JsonArray>();
    for (const auto &day: heatingHistory.getDayHistory()) {
        JsonObject dayObject = daysArray.add<JsonObject>();
        dayObject["day"] = day.day;       // 1-31
        dayObject["month"] = day.month;   // 1-12
        dayObject["year"] = day.year;     // Full year, e.g., 2024
        dayObject["total"] = day.total;
        JsonArray historyArray = dayObject["history"].to<JsonArray>();
        for (const auto &hourlyValue: day.history) {
            historyArray.add(hourlyValue);
        }
    }
    JsonArray monthsArray = doc["months"].to<JsonArray>();
    for (const auto &month: heatingHistory.getMonthHistory()) {
        JsonObject monthObject = monthsArray.add<JsonObject>();
        monthObject["month"] = month.month; // 1-12
        monthObject["year"] = month.year;   // Full year
        monthObject["total"] = month.total;
        JsonArray historyArray = monthObject["history"].to<JsonArray>();
        for (const auto &dailyValue: month.history) {
            historyArray.add(dailyValue);
        }
    }
    JsonArray yearsArray = doc["years"].to<JsonArray>();
    for (const auto &year: heatingHistory.getYearHistory()) {
        JsonObject yearObject = yearsArray.add<JsonObject>();
        yearObject["year"] = year.year;   // Full year
        yearObject["total"] = year.total;
        JsonArray historyArray = yearObject["history"].to<JsonArray>();
        for (const auto &monthlyValue: year.history) {
            historyArray.add(monthlyValue);
        }
    }
    JsonArray runTimesArray = doc["run_times"].to<JsonArray>();
    for (const auto &run: heatingHistory.getRunTimes()) {
        JsonObject runObject = runTimesArray.add<JsonObject>();
        runObject["start"] = run.start;
        runObject["end"] = run.end;
        JsonArray roomsArray = runObject["rooms"].to<JsonArray>();
        for (const auto &room: run.roomsData) {
            JsonObject roomObject = roomsArray.add<JsonObject>();
            roomObject["name"] = room.name;
            roomObject["start_temperature"] = room.startTemperature;
            roomObject["end_temperature"] = room.endTemperature;
            roomObject["start_humidity"] = room.startHumidity;
            roomObject["end_humidity"] = room.endHumidity;
            roomObject["priority"] = room.priority;
        }
    }
    doc["time"] = time(nullptr);
    if (serializeJson(doc, file) == 0) {
        Serial.println("Failed to write to file");
    } else {
        Serial.println("History successfully saved.");
    }
    file.close();
}

void loadHistory() {
    if (!LittleFS.exists("/history.json")) {
        Serial.println("History file does not exist. Creating new file.");
        saveHistory();
        return;
    }
    File file = LittleFS.open("/history.json", "r");
    if (!file) {
        Serial.println("There was an error opening the file for reading");
        return;
    }
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, file);
    if (error) {
        Serial.print("Failed to read file, using default configuration: ");
        Serial.println(error.f_str());
        file.close();
        return;
    }
    std::vector<DayWithDetails> days;
    if (doc["days"].is<JsonArray>()) {
        JsonArray daysArray = doc["days"].as<JsonArray>();
        for (const auto &dayObject: daysArray) {
            if (!dayObject.is<JsonObject>()) continue;
            DayWithDetails day;
            day.day = dayObject["day"].as<int>();       // 1-31
            day.month = dayObject["month"].as<int>();   // 1-12
            day.year = dayObject["year"].as<int>();     // Full year
            day.total = dayObject["total"].as<unsigned long>();
            if (dayObject["history"].is<JsonArray>()) {
                JsonArray historyArray = dayObject["history"].as<JsonArray>();
                size_t index = 0;
                for (const auto &hourlyValue: historyArray) {
                    if (index >= day.history.size()) break;
                    day.history[index++] = hourlyValue.as<unsigned long>();
                }
            }
            days.push_back(day);
        }
    }
    std::vector<MonthWithDetails> months;
    if (doc["months"].is<JsonArray>()) {
        JsonArray monthsArray = doc["months"].as<JsonArray>();
        for (const auto &monthObject: monthsArray) {
            if (!monthObject.is<JsonObject>()) continue;
            MonthWithDetails month;
            month.month = monthObject["month"].as<int>(); // 1-12
            month.year = monthObject["year"].as<int>();   // Full year
            month.total = monthObject["total"].as<unsigned long>();
            if (monthObject["history"].is<JsonArray>()) {
                JsonArray historyArray = monthObject["history"].as<JsonArray>();
                size_t index = 0;
                for (const auto &dailyValue: historyArray) {
                    if (index >= month.history.size()) break;
                    month.history[index++] = dailyValue.as<unsigned long>();
                }
            }
            months.push_back(month);
        }
    }
    std::vector<YearWithDetails> years;
    if (doc["years"].is<JsonArray>()) {
        JsonArray yearsArray = doc["years"].as<JsonArray>();
        for (const auto &yearObject: yearsArray) {
            if (!yearObject.is<JsonObject>()) continue;
            YearWithDetails year;
            year.year = yearObject["year"].as<int>();
            year.total = yearObject["total"].as<unsigned long>();
            if (yearObject["history"].is<JsonArray>()) {
                JsonArray historyArray = yearObject["history"].as<JsonArray>();
                size_t index = 0;
                for (const auto &monthlyValue: historyArray) {
                    if (index >= year.history.size()) break;
                    year.history[index++] = monthlyValue.as<unsigned long>();
                }
            }
            years.push_back(year);
        }
    }
    std::vector<RunTime> runTimes;
    if (doc["run_times"].is<JsonArray>()) {
        JsonArray runTimesArray = doc["run_times"].as<JsonArray>();
        for (const auto &runObject: runTimesArray) {
            if (!runObject.is<JsonObject>()) continue;
            RunTime run;
            run.start = runObject["start"].as<long>();
            run.end = runObject["end"].as<long>();
            if (runObject["rooms"].is<JsonArray>()) {
                JsonArray roomsArray = runObject["rooms"].as<JsonArray>();
                for (const auto &roomObject: roomsArray) {
                    if (!roomObject.is<JsonObject>()) continue;
                    RoomData room;
                    room.name = roomObject["name"].as<const char *>();
                    room.startTemperature = roomObject["start_temperature"].as<float>();
                    room.endTemperature = roomObject["end_temperature"].as<float>();
                    room.startHumidity = roomObject["start_humidity"].as<float>();
                    room.endHumidity = roomObject["end_humidity"].as<float>();
                    room.priority = roomObject["priority"].as<float>();
                    run.roomsData.push_back(room);
                }
            }
            runTimes.push_back(run);
        }
    }
    heatingHistory = HeatingHistory(runTimes, days, months, years);
    Serial.println("History successfully loaded.");
    file.close();
}

void saveHeatingMode() {
    if (!LittleFS.exists("/heatingMode.json")) {
        Serial.println("Heating mode file does not exist. Creating new file.");
        LittleFS.open("/heatingMode.json", "w").close();
    }
    File file = LittleFS.open("/heatingMode.json", "w");
    if (!file) {
        Serial.println("There was an error opening the file for writing");
        return;
    }
    JsonDocument doc;
    if (heatingMode == AUTO) {
        doc["heatingMode"] = "AUTO";
    } else if (heatingMode == MANUAL) {
        doc["heatingMode"] = "MANUAL";
    } else if (heatingMode == OFF) {
        doc["heatingMode"] = "OFF";
    } else {
        doc["heatingMode"] = "AUTO";
    }
    if (manualMode == OFF_MANUAL) {
        doc["manualMode"] = "OFF";
    } else if (manualMode == ON_MANUAL) {
        doc["manualMode"] = "ON";
    } else {
        doc["manualMode"] = "OFF";
    }
    if (serializeJson(doc, file) == 0) {
        Serial.println("Failed to write to file");
    }
    file.close();
}

void loadHeatingMode() {
    if (!LittleFS.exists("/heatingMode.json")) {
        Serial.println("Heating mode file does not exist. Creating new file.");
        saveHeatingMode();
    }
    File file = LittleFS.open("/heatingMode.json", "r");
    if (!file) {
        Serial.println("There was an error opening the file for reading");
        return;
    }
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, file);
    if (error) {
        Serial.print("Failed to read file, using default configuration: ");
        Serial.println(error.f_str());
        file.close();
        return;
    }
    if (doc["heatingMode"] == "AUTO") {
        heatingMode = AUTO;
    } else if (doc["heatingMode"] == "MANUAL") {
        heatingMode = MANUAL;
    } else if (doc["heatingMode"] == "OFF") {
        heatingMode = OFF;
    } else {
        heatingMode = AUTO;
    }
    if (doc["manualMode"] == "OFF") {
        manualMode = OFF_MANUAL;
    } else if (doc["manualMode"] == "ON") {
        manualMode = ON_MANUAL;
    } else {
        manualMode = OFF_MANUAL;
    }
    file.close();
}