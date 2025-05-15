#ifndef ESP32_TERMOSTAT_OTAUPDATE_H
#define ESP32_TERMOSTAT_OTAUPDATE_H

#include <ArduinoOTA.h>

void setupOTAUpdate();

void otaUpdateTask(void *parameter);

void startOTAUpdate();

#endif
