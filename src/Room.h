#ifndef ESP32_TERMOSTAT_ROOM_H
#define ESP32_TERMOSTAT_ROOM_H

#include <string>
#include <vector>
#include <memory> // Adăugat pentru smart pointers
#include "ATC_MiThermometer.h"
#include <cstdint>

enum themperature_modes {
    HOME,
    AWAY,
    NIGHT,
    ANTIFREEZE
};

class Room {
private:
    std::string room_name;
    std::vector<std::shared_ptr<ATC_MiThermometer>> thermometers; // Modificat pentru a folosi shared_ptr
    float home_target_temperature;
    float home_low_offset;
    float home_high_offset;
    float room_priority;
    float away_target_temperature;
    float away_low_offset;
    float away_high_offset;
    float night_target_temperature;
    float night_low_offset;
    float night_high_offset;
    float temperature{};
    float humidity{};
    static themperature_modes mode;
public:
    void addThermometer(std::string mac, bool load = false);

    float getRoomTemperature();

    float get_temperature_needs();

    bool valid_thermometers();

    void removeThermometer(std::string mac, bool load = false);

    void set_room_name(std::string new_name, bool load = false);

    std::string get_room_name() const;

    void set_home_temperature(float new_temperature, bool load = false);

    float get_home_temperature() const;

    void set_home_low_offset(float new_low_offset, bool load = false);

    float get_home_low_offset() const;

    void set_home_high_offset(float new_high_offset, bool load = false);

    float get_home_high_offset() const;

    void set_away_temperature(float new_temperature, bool load = false);

    float get_away_temperature() const;

    void set_away_low_offset(float new_low_offset, bool load = false);

    float get_away_low_offset() const;

    void set_away_high_offset(float new_high_offset, bool load = false);

    float get_away_high_offset() const;

    void set_night_temperature(float new_temperature, bool load = false);

    float get_night_temperature() const;

    void set_night_low_offset(float new_low_offset, bool load = false);

    float get_night_low_offset() const;

    void set_night_high_offset(float new_high_offset, bool load = false);

    float get_night_high_offset() const;

    void set_room_priority(float new_priority, bool load = false);

    float get_room_priority() const;

    static void set_room_mode(themperature_modes mode, bool load = false);

    static themperature_modes get_room_mode();

    uint8_t get_thermometer_number();

    bool thermometerExist(std::string mac);

    explicit Room(std::string room_name, bool load = false);

    std::string get_mac_by_index(int index);

    float get_temperature_by_index(int index);

    float get_humidity_by_index(int index);

    float get_humidity();

    int get_battery_mv_by_index(int index);

    int get_battery_percent_by_index(int index);

    bool get_valid_by_index(int index);

    Room(std::string room_name, float home_target_temperature, float home_low_offset, float home_high_offset,
         float room_priority, float away_target_temperature, float away_low_offset, float away_high_offset,
         float night_target_temperature, float night_low_offset, float night_high_offset, bool load = false);

    void calculateRoomTemperature();

    void calculateRoomHumidity();
};

#endif // ESP32_TERMOSTAT_ROOM_H
