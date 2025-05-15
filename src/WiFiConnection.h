#ifndef ESP32_TERMOSTAT_WIFICONNECTION_H
#define ESP32_TERMOSTAT_WIFICONNECTION_H

#include <WiFi.h>
#include "wifi_credentials.h"   // ← added

// NTP Configuration
extern const char *NTP_SERVER;         // NTP server address
extern const long GMT_OFFSET_SEC;      // GMT offset in seconds (e.g., GMT+2 for Romania)
extern const int DAYLIGHT_OFFSET_SEC;  // Daylight offset in seconds
extern const unsigned long NTP_UPDATE_INTERVAL_MS; // NTP update interval in milliseconds (1 hour)

// NTP Retry Configuration
extern const int NTP_MAX_RETRIES;              // Maximum number of NTP synchronization attempts
extern const unsigned long NTP_RETRY_DELAY_MS; // Delay between NTP retries (10 seconds)

// WiFi Retry Configuration
extern const int MAX_WIFI_RETRIES;                  // Maximum number of WiFi reconnection attempts
extern const unsigned long WIFI_RETRY_DELAY_MS;      // Delay between reconnection attempts (5 seconds)
extern const unsigned long WIFI_LONG_RETRY_DELAY_MS; // Delay after max retries before next reconnection attempt (5 minutes)

// Function Prototypes
void initWiFi(bool useAPMode);  // Added parameter to choose between AP mode and Station mode
void WiFiEventHandler(WiFiEvent_t event, WiFiEventInfo_t info);

#endif // ESP32_TERMOSTAT_WIFICONNECTION_H
