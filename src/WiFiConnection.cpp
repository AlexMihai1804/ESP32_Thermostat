#include "wifi_credentials.h"   // ← added
#include "WIFiConnection.h"
#include <ESPmDNS.h>
#include <WiFi.h>

// NTP Configuration
const char *NTP_SERVER = "pool.ntp.org";    // NTP server address
const long GMT_OFFSET_SEC = 7200;           // GMT offset in seconds (e.g., GMT+2 for Romania)
const int DAYLIGHT_OFFSET_SEC = 3600;       // Daylight offset in seconds
const unsigned long NTP_UPDATE_INTERVAL_MS = 3600000; // NTP update interval in milliseconds (1 hour)

// NTP Retry Configuration
const int NTP_MAX_RETRIES = 5;                  // Maximum number of NTP synchronization attempts
const unsigned long NTP_RETRY_DELAY_MS = 10000; // Delay between NTP retries (10 seconds)

// WiFi Retry Configuration
const int MAX_WIFI_RETRIES = 5;                   // Maximum number of WiFi reconnection attempts
const unsigned long WIFI_RETRY_DELAY_MS = 5000;   // Delay between reconnection attempts (5 seconds)
const unsigned long WIFI_LONG_RETRY_DELAY_MS = 300000; // Delay after max retries before next reconnection attempt (5 minutes)

// Global Variables
static int wifiRetryCount = 0;                // Current WiFi reconnection attempt count
static TaskHandle_t ntpTaskHandle = NULL;     // Handle for the NTP update task
static bool shouldReconnect = true;           // Flag to control reconnection attempts
static bool isAPMode = false;                 // Flag to indicate if the device is in AP mode

static void connectToWiFi();

static void handleWiFiConnectionFailure();

static void wifiLongRetryTask(void *parameter);

static void synchronizeTime();

static bool attemptSynchronizeTime();

static void ntpUpdateTask(void *parameter);

void initWiFi(bool useAPMode) {
    isAPMode = useAPMode;
    Serial.println("Initializing WiFi connection...");
    WiFi.onEvent(WiFiEventHandler);

    if (isAPMode) {
        // Set up Access Point mode
        Serial.println("Starting in Access Point (AP) mode...");
        WiFi.mode(WIFI_AP);
        WiFi.softAP(WIFI_SSID, WIFI_PASSWORD);
        IPAddress IP = WiFi.softAPIP();
        Serial.print("AP IP address: ");
        Serial.println(IP);

        // Set up mDNS responder
        if (!MDNS.begin("termostat")) {
            Serial.println("Error setting up mDNS responder!");
        } else {
            Serial.println("mDNS responder started: thermostat.local");
            MDNS.addService("http", "tcp", 443);  // Optional, add an HTTP service on port 80
        }

    } else {
        // Set up Station mode
        Serial.println("Starting in Station mode...");
        connectToWiFi();
    }

    // Start NTP update task
    xTaskCreatePinnedToCore(
            ntpUpdateTask,           // Task function
            "NTP Update Task",       // Task name
            4096,                    // Stack size (in words)
            NULL,                    // Task input parameter
            1,                       // Priority
            &ntpTaskHandle,          // Task handle
            1                        // Core where the task should run
    );
}

// WiFi Event Handler
void WiFiEventHandler(WiFiEvent_t event, WiFiEventInfo_t info) {
    if (isAPMode) {
        // Handle events specific to AP mode if needed
        return;
    }

    switch (event) {
        case ARDUINO_EVENT_WIFI_STA_CONNECTED:
            Serial.println("WiFi connected.");
            break;
        case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
            Serial.println("WiFi disconnected. Attempting to reconnect...");
            if (shouldReconnect) {
                connectToWiFi();
            }
            break;
        case ARDUINO_EVENT_WIFI_STA_GOT_IP:
            Serial.print("Obtained IP Address: ");
            Serial.println(WiFi.localIP());
            wifiRetryCount = 0;
            synchronizeTime();
            break;
        default:
            break;
    }
}

// Connect to WiFi with retry logic
static void connectToWiFi() {
    if (wifiRetryCount < MAX_WIFI_RETRIES) {
        Serial.print("Connecting to WiFi: ");
        Serial.println(WIFI_SSID);
        WiFi.mode(WIFI_STA);
        WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
        wifiRetryCount++;
    } else {
        Serial.println("Max WiFi reconnection attempts reached.");
        handleWiFiConnectionFailure();
        wifiRetryCount = 0;
    }
}

// Handle WiFi connection failure by scheduling a long retry task
static void handleWiFiConnectionFailure() {
    Serial.println("Handling WiFi connection failure.");
    xTaskCreatePinnedToCore(
            wifiLongRetryTask,          // Task function
            "WiFi Long Retry Task",     // Task name
            2048,                       // Stack size (in words)
            NULL,                       // Task input parameter
            1,                          // Priority
            NULL,                       // Task handle
            1                           // Core where the task should run
    );
}

// Long retry task for WiFi reconnection after a delay
static void wifiLongRetryTask(void *parameter) {
    Serial.println("Waiting for 5 minutes before retrying WiFi connection...");
    vTaskDelay(pdMS_TO_TICKS(WIFI_LONG_RETRY_DELAY_MS));
    Serial.println("Retrying WiFi connection...");
    connectToWiFi();
    vTaskDelete(NULL);
}

// Synchronize time with NTP server with retry mechanism
static void synchronizeTime() {
    Serial.println("Synchronizing time with NTP server...");
    configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, NTP_SERVER);
    bool success = attemptSynchronizeTime();
    if (success) {
        Serial.println("Time synchronized successfully.");
    } else {
        Serial.println("Failed to synchronize time after maximum retries.");
    }
}

// Attempt to synchronize time with retries
static bool attemptSynchronizeTime() {
    struct tm timeInfo;
    for (int attempt = 1; attempt <= NTP_MAX_RETRIES; attempt++) {
        if (getLocalTime(&timeInfo)) {
            Serial.print("Time synchronized successfully on attempt ");
            Serial.println(attempt);
            Serial.println(&timeInfo, "%A, %d %B %Y %H:%M:%S");
            return true;
        } else {
            Serial.print("Attempt ");
            Serial.print(attempt);
            Serial.println(" failed to obtain time from NTP server.");
            if (attempt < NTP_MAX_RETRIES) {
                Serial.print("Retrying in ");
                Serial.print(NTP_RETRY_DELAY_MS / 1000);
                Serial.println(" seconds...");
                vTaskDelay(pdMS_TO_TICKS(NTP_RETRY_DELAY_MS));
            }
        }
    }
    return false;
}

// NTP Update Task with retry mechanism
static void ntpUpdateTask(void *parameter) {
    while (true) {
        vTaskDelay(pdMS_TO_TICKS(NTP_UPDATE_INTERVAL_MS)); // Wait for the update interval
        if (isAPMode) {
            Serial.println("NTP update skipped: Device is in AP mode.");
            continue;
        }
        if (WiFi.status() == WL_CONNECTED) {
            Serial.println("Performing periodic NTP update...");
            bool success = attemptSynchronizeTime();
            if (!success) {
                Serial.println("NTP synchronization failed after maximum retries.");
            }
        } else {
            Serial.println("Cannot perform NTP update: WiFi is not connected.");
        }
    }
}
