#include <string>
#include <utility>
#include <cmath>
#include <memory>
#include "Room.h"
#include "SaveLoad.h"
#include "globalSettings.h"

themperature_modes Room::mode = HOME;

Room::Room(std::string room_name, bool load) {
    this->room_name = std::move(room_name);
    this->home_target_temperature = 22.0f;
    this->home_high_offset = 0.5f;
    this->home_low_offset = 0.5f;
    this->away_target_temperature = 18.0f;
    this->away_high_offset = 0.75f;
    this->away_low_offset = 0.75f;
    this->night_target_temperature = 21.0f;
    this->night_high_offset = 0.6f;
    this->night_low_offset = 0.6f;
    this->room_priority = 5;
    if (!load) {
        saveRooms();
    }
}

Room::Room(std::string room_name, float home_target_temperature, float home_low_offset, float home_high_offset,
           float room_priority, float away_target_temperature, float away_low_offset, float away_high_offset,
           float night_target_temperature, float night_low_offset, float night_high_offset, bool load) {
    this->room_name = std::move(room_name);
    this->home_target_temperature = home_target_temperature;
    this->home_high_offset = home_high_offset;
    this->home_low_offset = home_low_offset;
    this->away_target_temperature = away_target_temperature;
    this->away_high_offset = away_high_offset;
    this->away_low_offset = away_low_offset;
    this->night_target_temperature = night_target_temperature;
    this->night_high_offset = night_high_offset;
    this->night_low_offset = night_low_offset;
    this->room_priority = room_priority;
    if (!load) {
        saveRooms();
    }
}

void Room::addThermometer(std::string mac, bool load) {
    auto thermometer = std::make_shared<ATC_MiThermometer>(mac);
    thermometer->setTimeTracking(true);
    bleAdvertisingReader.addThermometer(thermometer.get());
    this->thermometers.push_back(thermometer);
    if (!load) {
        saveRooms();
    }
}

void Room::calculateRoomTemperature() {
    float total = 0;
    uint8_t count = 0;
    for (auto &thermometer: this->thermometers) {
        if (time(nullptr) - thermometer->getLastReadTime() < 60) {
            total += thermometer->getTemperaturePrecise();
            count++;
        }
    }
    if (count > 0) {
        this->temperature = total / static_cast<float>(count);
    } else {
        this->temperature = 0;
    }
}

float Room::getRoomTemperature() {
    this->calculateRoomTemperature();
    return this->temperature;
}

float Room::get_temperature_needs() {
    float current_temp = this->getRoomTemperature();
    if (mode == HOME) {
        float delta_temp = current_temp - this->home_target_temperature;
        int sign = delta_temp < 0 ? -1 : 1;
        delta_temp = std::abs(delta_temp);
        float tempCoefficient = delta_temp / (sign == 1 ? this->home_high_offset : this->home_low_offset);
        if (tempCoefficient <= 1) {
            return tempCoefficient * this->room_priority * (float) sign;
        } else {
            return tempCoefficient * tempCoefficient * this->room_priority * (float) sign;
        }
    } else if (mode == AWAY) {
        float delta_temp = current_temp - this->away_target_temperature;
        int sign = delta_temp < 0 ? -1 : 1;
        delta_temp = std::abs(delta_temp);
        float tempCoefficient = delta_temp / (sign == 1 ? this->away_high_offset : this->away_low_offset);
        if (tempCoefficient <= 1) {
            return tempCoefficient * this->room_priority * (float) sign;
        } else {
            return tempCoefficient * tempCoefficient * this->room_priority * (float) sign;
        }
    } else if (mode == NIGHT) {
        float delta_temp = current_temp - this->night_target_temperature;
        int sign = delta_temp < 0 ? -1 : 1;
        delta_temp = std::abs(delta_temp);
        float tempCoefficient = delta_temp / (sign == 1 ? this->night_high_offset : this->night_low_offset);
        if (tempCoefficient <= 1) {
            return tempCoefficient * this->room_priority * (float) sign;
        } else {
            return tempCoefficient * tempCoefficient * this->room_priority * (float) sign;
        }
    } else if (mode == ANTIFREEZE) {
        if (current_temp < 5) {
            return 99999999.0f;
        }
    }
    return 0;
}

bool Room::valid_thermometers() {
    for (auto &thermometer: this->thermometers) {
        if (time(nullptr) - thermometer->getLastReadTime() < 60) return true;
    }
    return false;
}

void Room::removeThermometer(const std::string mac, bool load) {
    for (auto it = this->thermometers.begin(); it != this->thermometers.end(); ++it) {
        if ((*it)->getAddressString() == mac) {
            bleAdvertisingReader.removeThermometer(it->get());
            this->thermometers.erase(it);
            break;
        }
    }
    if (!load) {
        saveRooms();
    }
}

void Room::set_room_name(std::string new_name, bool load) {
    this->room_name = std::move(new_name);
    if (!load) {
        saveRooms();
    }
}

std::string Room::get_room_name() const {
    return this->room_name;
}

void Room::set_home_temperature(float new_temperature, bool load) {
    this->home_target_temperature = new_temperature;
    if (!load) {
        saveRooms();
    }
}

float Room::get_home_temperature() const {
    return this->home_target_temperature;
}

void Room::set_home_low_offset(float new_low_offset, bool load) {
    this->home_low_offset = new_low_offset;
    if (!load) {
        saveRooms();
    }
}

float Room::get_home_low_offset() const {
    return this->home_low_offset;
}

void Room::set_home_high_offset(float new_high_offset, bool load) {
    this->home_high_offset = new_high_offset;
    if (!load) {
        saveRooms();
    }
}

float Room::get_home_high_offset() const {
    return this->home_high_offset;
}

void Room::set_away_temperature(float new_temperature, bool load) {
    this->away_target_temperature = new_temperature;
    if (!load) {
        saveRooms();
    }
}

float Room::get_away_temperature() const {
    return this->away_target_temperature;
}

void Room::set_away_low_offset(float new_low_offset, bool load) {
    this->away_low_offset = new_low_offset;
    if (!load) {
        saveRooms();
    }
}

float Room::get_away_low_offset() const {
    return this->away_low_offset;
}

void Room::set_away_high_offset(float new_high_offset, bool load) {
    this->away_high_offset = new_high_offset;
    if (!load) {
        saveRooms();
    }
}

float Room::get_away_high_offset() const {
    return this->away_high_offset;
}

void Room::set_night_temperature(float new_temperature, bool load) {
    this->night_target_temperature = new_temperature;
    if (!load) {
        saveRooms();
    }
}

float Room::get_night_temperature() const {
    return this->night_target_temperature;
}

void Room::set_night_low_offset(float new_low_offset, bool load) {
    this->night_low_offset = new_low_offset;
    if (!load) {
        saveRooms();
    }
}

float Room::get_night_low_offset() const {
    return this->night_low_offset;
}

void Room::set_night_high_offset(float new_high_offset, bool load) {
    this->night_high_offset = new_high_offset;
    if (!load) {
        saveRooms();
    }
}

float Room::get_night_high_offset() const {
    return this->night_high_offset;
}

void Room::set_room_priority(float new_priority, bool load) {
    this->room_priority = new_priority;
    if (!load) {
        saveRooms();
    }
}

float Room::get_room_priority() const {
    return this->room_priority;
}

void Room::set_room_mode(themperature_modes new_mode, bool load) {
    mode = new_mode;
}

themperature_modes Room::get_room_mode() {
    return mode;
}

uint8_t Room::get_thermometer_number() {
    return this->thermometers.size();
}

bool Room::thermometerExist(std::string mac) {
    for (auto &thermometer: this->thermometers) {
        if (thermometer->getAddressString() == mac) {
            return true;
        }
    }
    return false;
}

float Room::get_temperature_by_index(int index) {
    if (index >= 0 && index < this->thermometers.size()) {
        if (time(nullptr) - this->thermometers[index]->getLastReadTime() < 60) {
            return this->thermometers[index]->getTemperaturePrecise();
        } else{
            Serial.print("Temperature is not valid");
            Serial.println(time(nullptr) - this->thermometers[index]->getLastReadTime());
        }
    }
    return 0.0f;
}

float Room::get_humidity_by_index(int index) {
    if (index >= 0 && index < this->thermometers.size()) {
        if (time(nullptr) - this->thermometers[index]->getLastReadTime() < 60) {
            return this->thermometers[index]->getHumidity();
        }
    }
    return 0.0f;
}

int Room::get_battery_mv_by_index(int index) {
    if (index >= 0 && index < this->thermometers.size()) {
        return this->thermometers[index]->getBatteryVoltage();
    }
    return 0; // Sau gestionează eroarea corespunzător
}

int Room::get_battery_percent_by_index(int index) {
    if (index >= 0 && index < this->thermometers.size()) {
        return this->thermometers[index]->getBatteryLevel();
    }
    return 0; // Sau gestionează eroarea corespunzător
}

bool Room::get_valid_by_index(int index) {
    if (index >= 0 && index < this->thermometers.size()) {
        return time(nullptr) - this->thermometers[index]->getLastReadTime() < 60;
    }
    return false; // Sau gestionează eroarea corespunzător
}

void Room::calculateRoomHumidity() {
    float total = 0;
    int count = 0;
    for (auto &thermometer: this->thermometers) {
        if (time(nullptr) - thermometer->getLastReadTime() < 60) {
            total += thermometer->getHumidity();
            count++;
        }
    }
    if (count > 0) {
        this->humidity = total / static_cast<float>(count);
    } else {
        this->humidity = 0;
    }
}

float Room::get_humidity() {
    this->calculateRoomHumidity();
    return this->humidity;
}

std::string Room::get_mac_by_index(int index) {
    if (index >= 0 && index < this->thermometers.size()) {
        return this->thermometers[index]->getAddressString();
    }
    return ""; // Sau gestionează eroarea corespunzător
}
