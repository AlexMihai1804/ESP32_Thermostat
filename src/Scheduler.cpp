#include "Scheduler.h"
#include <utility>
#include <ctime>
#include <vector>
#include "globalSettings.h"
#include "SaveLoad.h"

Scheduler::Scheduler() {
    for (auto &i: this->schedule) {
        for (auto &j: i) {
            j = HOME;
        }
    }
}

Scheduler::Scheduler(themperature_modes (*schedule)[48], std::vector<directive> userDirectives,
                     std::vector<directive> smartDirectives, bool load) {
    for (int i = 0; i < 7; ++i) {
        for (int j = 0; j < 48; ++j) {
            this->schedule[i][j] = schedule[i][j];
        }
    }
    this->userDirectives = std::move(userDirectives);
    this->smartDirectives = std::move(smartDirectives);
    if (!load) {
        saveSchedule();
    }
}

float averageHeatingRate() {
    /*
    std::vector<runTime> runs = history.getRunTimes();
    if (runs.empty()) {
        return 999999;
    }
    float total = 0;
    for (auto &run: runs) {
        float averageInitialTemp = 0;
        float averageFinalTemp = 0;
        float runTimeMinutes = difftime(run.end, run.start) / 60;
        for (auto &room: run.roomsData) {
            averageInitialTemp += room.startTemperature;
            averageFinalTemp += room.endTemperature;
        }
        averageInitialTemp /= run.roomsData.size();
        averageFinalTemp /= run.roomsData.size();
        total += (averageFinalTemp - averageInitialTemp) / runTimeMinutes;
    }
    return total / runs.size();
    */
    return 0.1;
}

void Scheduler::updateSchedule() {
    this->updateSmartDirectives();
    this->updateUserDirectives();
    smartUpdate();
    int hour, minute, weekday;
    time_t now;
    tm timeinfo{};
    time(&now);
    localtime_r(&now, &timeinfo);
    hour = timeinfo.tm_hour;
    minute = timeinfo.tm_min;
    weekday = (timeinfo.tm_wday + 6) % 7;
    for (auto it = this->smartDirectives.rbegin(); it != this->smartDirectives.rend(); ++it) {
        if (it->finalTime >= now && it->startTime <= now) {
            Room::set_room_mode(it->mode);
            return;
        }
    }
    for (auto it = this->userDirectives.rbegin(); it != this->userDirectives.rend(); ++it) {
        if (it->finalTime >= now && it->startTime <= now) {
            Room::set_room_mode(it->mode);
            return;
        }
    }
    int index = hour * 2;
    if (minute >= 30) index++;
    Room::set_room_mode(this->schedule[weekday][index]);
}

void Scheduler::addUserDirective(time_t t1, time_t t2, themperature_modes m, bool load) {
    directive d = {t1, m, t2};
    this->userDirectives.push_back(d);
    if (!load) {
        saveSchedule();
    }
}

void Scheduler::addSmartDirective(time_t t1, time_t t2, themperature_modes m, bool load) {
    directive d = {t1, m, t2};
    this->smartDirectives.push_back(d);
    if (!load) {
        saveSchedule();
    }
}

void Scheduler::updateSmartDirectives() {
    time_t now;
    time(&now);
    for (int i = 0; i < this->smartDirectives.size(); ++i) {
        if (this->smartDirectives[i].finalTime < now) { // Corrected condition
            this->smartDirectives.erase(this->smartDirectives.begin() + i);
            i--;
        }
    }
}

void Scheduler::updateUserDirectives() {
    time_t now;
    time(&now);
    for (int i = 0; i < this->userDirectives.size(); ++i) {
        if (this->userDirectives[i].finalTime < now) { // Corrected condition
            this->userDirectives.erase(this->userDirectives.begin() + i);
            i--;
        }
    }
}

void Scheduler::getHomeAtTime(time_t arrivalTime) {
    time_t endTime;
    int hour, minute, weekday, day, mon, year;
    tm timeinfo{};
    localtime_r(&arrivalTime, &timeinfo);
    hour = timeinfo.tm_hour;
    minute = timeinfo.tm_min;
    weekday = (timeinfo.tm_wday + 6) % 7;
    day = timeinfo.tm_mday;
    mon = timeinfo.tm_mon;
    year = timeinfo.tm_year;
    int index = hour * 2;
    if (minute >= 30) index++;
    for (int i = weekday; i < 7; ++i) {
        for (int j = index; j < 48; ++j) {
            if (this->schedule[i][j] == HOME) {
                tm tEnd{};
                tEnd.tm_year = year;
                tEnd.tm_mon = mon;
                tEnd.tm_mday = day;
                tEnd.tm_hour = j / 2;
                tEnd.tm_min = (j % 2) * 30;
                tEnd.tm_sec = 0;
                tEnd.tm_isdst = -1; // Initialize tm_isdst
                endTime = mktime(&tEnd);
                endTime += 60 * 60 * 24 * (i - weekday);
                this->addUserDirective(arrivalTime, endTime, HOME);
                return;
            }
        }
        index = 0;
    }
    for (int i = 0; i <= weekday; ++i) {
        for (int j = 0; j < 48; ++j) {
            if (this->schedule[i][j] == HOME) {
                tm tEnd{};
                tEnd.tm_year = year;
                tEnd.tm_mon = mon;
                tEnd.tm_mday = day;
                tEnd.tm_hour = j / 2;
                tEnd.tm_min = (j % 2) * 30;
                tEnd.tm_sec = 0;
                tEnd.tm_isdst = -1; // Initialize tm_isdst
                endTime = mktime(&tEnd);
                endTime += 60 * 60 * 24 * (i + 7 - weekday);
                this->addUserDirective(arrivalTime, endTime, HOME);
                return;
            }
        }
    }
}

void Scheduler::getHomeNow() {
    time_t endTime, now;
    int hour, minute, weekday, day, mon, year;
    tm timeinfo{};
    time(&now);
    localtime_r(&now, &timeinfo);
    hour = timeinfo.tm_hour;
    minute = timeinfo.tm_min;
    weekday = (timeinfo.tm_wday + 6) % 7;
    day = timeinfo.tm_mday;
    mon = timeinfo.tm_mon;
    year = timeinfo.tm_year;
    int index = hour * 2;
    if (minute >= 30) index++;
    for (int i = weekday; i < 7; ++i) {
        for (int j = index; j < 48; ++j) {
            if (this->schedule[i][j] == HOME) {
                tm tEnd{};
                tEnd.tm_year = year;
                tEnd.tm_mon = mon;
                tEnd.tm_mday = day;
                tEnd.tm_hour = j / 2;
                tEnd.tm_min = (j % 2) * 30;
                tEnd.tm_sec = 0;
                tEnd.tm_isdst = -1; // Initialize tm_isdst
                endTime = mktime(&tEnd);
                endTime += 60 * 60 * 24 * (i - weekday);
                this->addUserDirective(now, endTime, HOME);
                return;
            }
        }
        index = 0;
    }
    for (int i = 0; i <= weekday; ++i) {
        for (int j = 0; j < 48; ++j) {
            if (this->schedule[i][j] == HOME) {
                tm tEnd{};
                tEnd.tm_year = year;
                tEnd.tm_mon = mon;
                tEnd.tm_mday = day;
                tEnd.tm_hour = j / 2;
                tEnd.tm_min = (j % 2) * 30;
                tEnd.tm_sec = 0;
                tEnd.tm_isdst = -1; // Initialize tm_isdst
                endTime = mktime(&tEnd);
                endTime += 60 * 60 * 24 * (i + 7 - weekday);
                this->addUserDirective(now, endTime, HOME);
                return;
            }
        }
    }
}

void Scheduler::leaveAtTime(time_t leaveTime) {
    time_t endTime;
    int hour, minute, weekday, day, mon, year;
    tm timeinfo{};
    localtime_r(&leaveTime, &timeinfo);
    hour = timeinfo.tm_hour;
    minute = timeinfo.tm_min;
    weekday = (timeinfo.tm_wday + 6) % 7;
    day = timeinfo.tm_mday;
    mon = timeinfo.tm_mon;
    year = timeinfo.tm_year;
    int index = hour * 2;
    if (minute >= 30) index++;
    for (int i = weekday; i < 7; ++i) {
        for (int j = index; j < 48; ++j) {
            if (this->schedule[i][j] == AWAY) {
                tm tEnd{};
                tEnd.tm_year = year;
                tEnd.tm_mon = mon;
                tEnd.tm_mday = day;
                tEnd.tm_hour = j / 2;
                tEnd.tm_min = (j % 2) * 30;
                tEnd.tm_sec = 0;
                tEnd.tm_isdst = -1; // Initialize tm_isdst
                endTime = mktime(&tEnd);
                endTime += 60 * 60 * 24 * (i - weekday);
                this->addUserDirective(leaveTime, endTime, AWAY);
                return;
            }
        }
        index = 0;
    }
    for (int i = 0; i <= weekday; ++i) {
        for (int j = 0; j < 48; ++j) {
            if (this->schedule[i][j] == AWAY) {
                tm tEnd{};
                tEnd.tm_year = year;
                tEnd.tm_mon = mon;
                tEnd.tm_mday = day;
                tEnd.tm_hour = j / 2;
                tEnd.tm_min = (j % 2) * 30;
                tEnd.tm_sec = 0;
                tEnd.tm_isdst = -1; // Initialize tm_isdst
                endTime = mktime(&tEnd);
                endTime += 60 * 60 * 24 * (i + 7 - weekday);
                this->addUserDirective(leaveTime, endTime, AWAY);
                return;
            }
        }
    }
}

void Scheduler::leaveNow() {
    time_t endTime, now;
    int hour, minute, weekday, day, mon, year;
    tm timeinfo{};
    time(&now);
    localtime_r(&now, &timeinfo);
    hour = timeinfo.tm_hour;
    minute = timeinfo.tm_min;
    weekday = (timeinfo.tm_wday + 6) % 7;
    day = timeinfo.tm_mday;
    mon = timeinfo.tm_mon;
    year = timeinfo.tm_year;
    int index = hour * 2;
    if (minute >= 30) index++;
    for (int i = weekday; i < 7; ++i) {
        for (int j = index; j < 48; ++j) {
            if (this->schedule[i][j] == AWAY) {
                tm tEnd{};
                tEnd.tm_year = year;
                tEnd.tm_mon = mon;
                tEnd.tm_mday = day;
                tEnd.tm_hour = j / 2;
                tEnd.tm_min = (j % 2) * 30;
                tEnd.tm_sec = 0;
                tEnd.tm_isdst = -1; // Initialize tm_isdst
                endTime = mktime(&tEnd);
                endTime += 60 * 60 * 24 * (i - weekday);
                this->addUserDirective(now, endTime, AWAY);
                return;
            }
        }
        index = 0;
    }
    for (int i = 0; i <= weekday; ++i) {
        for (int j = 0; j < 48; ++j) {
            if (this->schedule[i][j] == AWAY) {
                tm tEnd{};
                tEnd.tm_year = year;
                tEnd.tm_mon = mon;
                tEnd.tm_mday = day;
                tEnd.tm_hour = j / 2;
                tEnd.tm_min = (j % 2) * 30;
                tEnd.tm_sec = 0;
                tEnd.tm_isdst = -1; // Initialize tm_isdst
                endTime = mktime(&tEnd);
                endTime += 60 * 60 * 24 * (i + 7 - weekday);
                this->addUserDirective(now, endTime, AWAY);
                return;
            }
        }
    }
}

themperature_modes (*Scheduler::getSchedule())[48] {
    return schedule;
}

themperature_modes Scheduler::getScheduleAtTime(uint8_t day, uint8_t time) {
    return this->schedule[day][time];
}

std::vector<directive> Scheduler::getSmartDirectives() {
    return this->smartDirectives;
}

std::vector<directive> Scheduler::getUserDirectives() {
    return this->userDirectives;
}

directive Scheduler::getUserDirectiveAtIndex(uint8_t index) {
    return this->userDirectives[index];
}

uint8_t Scheduler::getUserDirectiveNumber() {
    return this->userDirectives.size();
}

directive Scheduler::getSmartDirectiveAtIndex(uint8_t index) {
    return this->smartDirectives[index];
}

uint8_t Scheduler::getSmartDirectiveNumber() {
    return this->smartDirectives.size();
}

void Scheduler::setScheduleAtTime(uint8_t day, uint8_t time, themperature_modes mode, bool load) {
    this->schedule[day][time] = mode;
    if (!load) {
        saveSchedule();
    }
}

void Scheduler::removeUserDirectiveAtIndex(uint8_t index) {
    this->userDirectives.erase(this->userDirectives.begin() + index);
    saveSchedule();
}

void Scheduler::removeSmartDirectiveAtIndex(uint8_t index) {
    this->smartDirectives.erase(this->smartDirectives.begin() + index);
    saveSchedule();
}

void Scheduler::removeUserDirective(time_t t1, time_t t2, themperature_modes m) {
    for (int i = 0; i < this->userDirectives.size(); ++i) {
        if (this->userDirectives[i].startTime == t1 && this->userDirectives[i].finalTime == t2 &&
            this->userDirectives[i].mode == m) {
            this->userDirectives.erase(this->userDirectives.begin() + i);
            saveSchedule();
            return;
        }
    }
}

void Scheduler::removeSmartDirective(time_t t1, time_t t2, themperature_modes m) {
    for (int i = 0; i < this->smartDirectives.size(); ++i) {
        if (this->smartDirectives[i].startTime == t1 && this->smartDirectives[i].finalTime == t2 &&
            this->smartDirectives[i].mode == m) {
            this->smartDirectives.erase(this->smartDirectives.begin() + i);
            saveSchedule();
            return;
        }
    }
}

themperature_modes Scheduler::getModeAtTime(time_t time) {
    for (auto it = this->smartDirectives.rbegin(); it != this->smartDirectives.rend(); ++it) {
        if (it->finalTime >= time && it->startTime <= time) {
            return it->mode;
        }
    }
    for (auto it = this->userDirectives.rbegin(); it != this->userDirectives.rend(); ++it) {
        if (it->finalTime >= time && it->startTime <= time) {
            return it->mode;
        }
    }
    tm timeinfo{};
    localtime_r(&time, &timeinfo);
    int hour = timeinfo.tm_hour;
    int minute = timeinfo.tm_min;
    int weekday = (timeinfo.tm_wday + 6) % 7;
    int index = hour * 2;
    if (minute >= 30) index++;
    return this->schedule[weekday][index];
}

time_t Scheduler::getNextChangeTime() {
    themperature_modes currentMode = getModeAtTime(time(nullptr));
    time_t now = time(nullptr);
    time_t lastDirectiveTime = now;
    for (auto it = this->smartDirectives.rbegin(); it != this->smartDirectives.rend(); ++it) {
        if (lastDirectiveTime < it->finalTime) {
            lastDirectiveTime = it->finalTime;
        }
    }
    for (auto it = this->userDirectives.rbegin(); it != this->userDirectives.rend(); ++it) {
        if (lastDirectiveTime < it->finalTime) {
            lastDirectiveTime = it->finalTime;
        }
    }
    time_t interval;
    if (lastDirectiveTime - now < 60 * 60 * 24 * 7) {
        interval = 60 * 60 * 24 * 7;
    } else {
        interval = lastDirectiveTime - now;
    }
    for (int i = 0; i < interval; i += 300) { // Increased step size to 5 minutes
        if (currentMode != getModeAtTime(now + i)) {
            return now + i;
        }
    }
    return now;
}

void Scheduler::smartUpdate() {
    if (rooms.empty()) {
        return;
    }
    time_t now = time(nullptr);
    time_t nextChange = getNextChangeTime();
    if (nextChange - now < 60) {
        return;
    }
    themperature_modes currentMode = getModeAtTime(now);
    themperature_modes nextMode = getModeAtTime(nextChange);
    if (currentMode == nextMode) {
        return;
    }
    if (currentMode == HOME) {
        return;
    }
    float rate = averageHeatingRate();
    if (currentMode == AWAY) {
        if (nextMode == HOME || nextMode == NIGHT) {
            float total = 0;
            float totalTarget = 0;
            for (auto &room: rooms) {
                total += room.getRoomTemperature();
                if (nextMode == HOME) {
                    totalTarget += room.get_home_temperature();
                } else if (nextMode == NIGHT) {
                    totalTarget += room.get_night_temperature();
                }
            }
            float average = total / rooms.size();
            float averageTarget = totalTarget / rooms.size();
            float timeToChange = (averageTarget - average) / rate;
            if (timeToChange * 60 > nextChange - now) {
                addSmartDirective(now, nextChange, nextMode);
            }
        } else {
            return;
        }
    }
    if (currentMode == NIGHT) {
        if (nextMode == HOME) {
            float total = 0;
            float totalTarget = 0;
            for (auto &room: rooms) {
                total += room.getRoomTemperature();
                totalTarget += room.get_home_temperature();
            }
            float average = total / rooms.size();
            float averageTarget = totalTarget / rooms.size();
            float timeToChange = (averageTarget - average) / rate;
            if (timeToChange * 60 > nextChange - now) {
                addSmartDirective(now, nextChange, HOME);
            }
        } else {
            return;
        }
    }
    if (currentMode == ANTIFREEZE) {
        if (nextMode == HOME || nextMode == NIGHT || nextMode == AWAY) {
            float total = 0;
            float totalTarget = 0;
            for (auto &room: rooms) {
                total += room.getRoomTemperature();
                if (nextMode == HOME) {
                    totalTarget += room.get_home_temperature();
                } else if (nextMode == NIGHT) {
                    totalTarget += room.get_night_temperature();
                } else {
                    totalTarget += room.get_away_temperature();
                }
            }
            float average = total / rooms.size();
            float averageTarget = totalTarget / rooms.size();
            float timeToChange = (averageTarget - average) / rate;
            if (timeToChange * 60 > nextChange - now) {
                addSmartDirective(now, nextChange, nextMode);
            }
        } else {
            return;
        }
    }
}