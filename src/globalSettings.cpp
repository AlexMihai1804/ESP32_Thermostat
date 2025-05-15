#include "globalSettings.h"

std::vector<Room> rooms;
Scheduler scheduler;
BLEAdvertisingReader bleAdvertisingReader;
HeatingHistory heatingHistory;
bool isHeating = false;
enum heatingMode heatingMode = AUTO;
enum manualMode manualMode = OFF_MANUAL;
portMUX_TYPE schedulerMux = portMUX_INITIALIZER_UNLOCKED;

// Add the missing schedulerMutex definition
SemaphoreHandle_t schedulerMutex = NULL;

SemaphoreHandle_t bleSemaphore = NULL; // Initialize to NULL before creating
portMUX_TYPE heatingMux = portMUX_INITIALIZER_UNLOCKED;

// Initialize task handles
TaskHandle_t webServerTaskHandle = NULL;
TaskHandle_t bleTaskHandle = NULL;
TaskHandle_t scheduleTaskHandle = NULL;
TaskHandle_t relayTaskHandle = NULL;

// Add flag for watchdog initialization
bool watchdogInitialized = false;

// Function to initialize all semaphores and mutexes
void initSemaphores() {
    // Initialize the scheduler mutex if not already initialized
    if (schedulerMutex == NULL) {
        schedulerMutex = xSemaphoreCreateMutex();
        if (schedulerMutex == NULL) {
            Serial.println("Failed to create scheduler mutex");
        }
    }
    
    // Initialize the BLE semaphore if not already initialized
    if (bleSemaphore == NULL) {
        bleSemaphore = xSemaphoreCreateMutex();
        if (bleSemaphore == NULL) {
            Serial.println("Failed to create BLE semaphore");
        }
    }
}
