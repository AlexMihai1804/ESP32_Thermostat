#ifndef ESP32_TERMOSTAT_SCHEDULER_H
#define ESP32_TERMOSTAT_SCHEDULER_H

#include "Room.h"

/**
 * @struct directive
 * @brief A structure to represent a directive.
 *
 * This structure provides fields to store the start time, mode, and final time of a directive.
 */
struct directive {
    time_t startTime; ///< The start time of the directive.
    themperature_modes mode; ///< The mode of the directive.
    time_t finalTime; ///< The final time of the directive.
};

/**
 * @class Scheduler
 * @brief A class to represent a Scheduler.
 *
 * This class provides methods to manage and update the schedule and directives.
 */
class Scheduler {
private:
    themperature_modes schedule[7][2 * 24]; ///< The schedule array.
    std::vector<directive> userDirectives; ///< The user directives.
    std::vector<directive> smartDirectives; ///< The smart directives.

public:
    /**
     * @brief Constructs a new Scheduler object.
     */
    Scheduler();

    /**
     * @brief Constructs a new Scheduler object with the given parameters.
     *
     * @param schedule The schedule array.
     * @param userDirectives The user directives.
     * @param smartDirectives The smart directives.
     * @param load A flag to indicate if the schedule should be loaded.
     */
    Scheduler(themperature_modes schedule[7][48], std::vector<directive> userDirectives,
              std::vector<directive> smartDirectives, bool load = false);

    /**
     * @brief Updates the schedule.
     */
    void updateSchedule();

    /**
     * @brief Adds a user directive.
     *
     * @param t1 The start time of the directive.
     * @param t2 The final time of the directive.
     * @param m The mode of the directive.
     * @param load A flag to indicate if the directive should be loaded.
     */
    void addUserDirective(time_t t1, time_t t2, themperature_modes m, bool load = false);

    /**
     * @brief Removes a user directive at the given index.
     *
     * @param index The index of the directive to remove.
     */
    void removeUserDirectiveAtIndex(uint8_t index);

    /**
     * @brief Removes a user directive.
     *
     * @param t1 The start time of the directive.
     * @param t2 The final time of the directive.
     * @param m The mode of the directive.
     */
    void removeUserDirective(time_t t1, time_t t2, themperature_modes m);

    /**
     * @brief Adds a smart directive.
     *
     * @param t1 The start time of the directive.
     * @param t2 The final time of the directive.
     * @param m The mode of the directive.
     * @param load A flag to indicate if the directive should be loaded.
     */
    void addSmartDirective(time_t t1, time_t t2, themperature_modes m, bool load = false);

    /**
     * @brief Removes a smart directive at the given index.
     *
     * @param index The index of the directive to remove.
     */
    void removeSmartDirectiveAtIndex(uint8_t index);

    /**
     * @brief Removes a smart directive.
     *
     * @param t1 The start time of the directive.
     * @param t2 The final time of the directive.
     * @param m The mode of the directive.
     */
    void removeSmartDirective(time_t t1, time_t t2, themperature_modes m);

    /**
     * @brief Updates the user directives.
     */
    void updateUserDirectives();

    /**
     * @brief Updates the smart directives.
     */
    void updateSmartDirectives();

    /**
     * @brief Gets the home at the given time.
     *
     * @param arrivalTime The arrival time.
     */
    void getHomeAtTime(time_t arrivalTime);

    /**
     * @brief Gets the home now.
     */
    void getHomeNow();

    /**
     * @brief Leaves at the given time.
     *
     * @param leaveTime The leave time.
     */
    void leaveAtTime(time_t leaveTime);

    /**
     * @brief Leaves now.
     */
    void leaveNow();

    /**
     * @brief Gets the schedule.
     *
     * @return The schedule.
     */
    themperature_modes (*getSchedule())[48];

    /**
     * @brief Gets the schedule at the given time.
     *
     * @param day The day of the schedule.
     * @param time The time of the schedule.
     * @return The mode at the given time.
     */
    themperature_modes getScheduleAtTime(uint8_t day, uint8_t time);

    /**
     * @brief Sets the schedule at the given time.
     *
     * @param day The day of the schedule.
     * @param time The time of the schedule.
     * @param mode The mode to set.
     * @param load A flag to indicate if the schedule should be loaded.
     */
    void setScheduleAtTime(uint8_t day, uint8_t time, themperature_modes mode, bool load = false);

    /**
     * @brief Gets the user directives.
     *
     * @return The user directives.
     */
    std::vector<directive> getUserDirectives();

    /**
     * @brief Gets the user directive at the given index.
     *
     * @param index The index of the directive.
     * @return The directive at the given index.
     */
    directive getUserDirectiveAtIndex(uint8_t index);

    /**
     * @brief Gets the number of user directives.
     *
     * @return The number of user directives.
     */
    uint8_t getUserDirectiveNumber();

    /**
     * @brief Gets the smart directives.
     *
     * @return The smart directives.
     */
    std::vector<directive> getSmartDirectives();

    /**
     * @brief Gets the smart directive at the given index.
     *
     * @param index The index of the directive.
     * @return The directive at the given index.
     */
    directive getSmartDirectiveAtIndex(uint8_t index);

    /**
     * @brief Gets the number of smart directives.
     *
     * @return The number of smart directives.
     */
    uint8_t getSmartDirectiveNumber();

    themperature_modes getModeAtTime(time_t time);

    time_t getNextChangeTime();

    void smartUpdate();
};

#endif //ESP32_TERMOSTAT_SCHEDULER_H
