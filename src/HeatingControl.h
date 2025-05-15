#ifndef ESP32_TERMOSTAT_HEATINGCONTROL_H
#define ESP32_TERMOSTAT_HEATINGCONTROL_H

#include <string>

enum heatingMode {
    AUTO,
    MANUAL,
    OFF
};
enum manualMode {
    OFF_MANUAL,
    ON_MANUAL
};
enum heatingStatus {
    START,
    NORMAL,
    STOP
};
struct RoomData {
    float startTemperature;
    float endTemperature;
    float startHumidity;
    float endHumidity;
    float priority;
    std::string name;
};

heatingStatus isHeatingNeeded();

void updateRelayStatus();

void relay_init();

void start_relay_sync();

void start_schedule_sync();

void update_schedule(void *pvParameters);

#endif //ESP32_TERMOSTAT_HEATINGCONTROL_H
