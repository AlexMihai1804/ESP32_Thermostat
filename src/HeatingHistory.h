#ifndef ESP32_TERMOSTAT_HEATINGHISTORY_H
#define ESP32_TERMOSTAT_HEATINGHISTORY_H

#include <cstdint>
#include <ctime>
#include <vector>
#include <unordered_map>
#include <array>
#include "HeatingControl.h"

// Forward declaration of RoomData if not included in HeatingControl.h
struct RoomData;

// Structure to represent a runtime period
struct RunTime {
    time_t start;
    time_t end;
    std::vector<RoomData> roomsData;
};

// Structure to hold detailed day information
struct DayWithDetails {
    std::uint8_t day;     // 1-31
    std::uint8_t month;   // 1-12
    std::uint16_t year;   // Full year, e.g., 2024
    std::uint32_t total;
    std::array<std::uint32_t, 24> history{}; // Hourly history
};

// Structure to represent a day key
struct Day {
    Day() = default;

    Day(std::uint8_t d, std::uint8_t m, std::uint16_t y) : day(d), month(m), year(y) {}

    std::uint8_t day;
    std::uint8_t month;
    std::uint16_t year;

    bool operator==(const Day &other) const {
        return day == other.day && month == other.month && year == other.year;
    }
};

// Structure to hold day details
struct DayDetails {
    std::uint32_t total = 0;
    std::array<std::uint32_t, 24> history{}; // Hourly history
};

// Custom hash function for Day
struct DayHash {
    size_t operator()(const Day &d) const {
        size_t res = std::hash<std::uint16_t>()(d.year);
        res ^= std::hash<std::uint8_t>()(d.month) + 0x9e3779b9 + (res << 6) + (res >> 2);
        res ^= std::hash<std::uint8_t>()(d.day) + 0x9e3779b9 + (res << 6) + (res >> 2);
        return res;
    }
};

// Structure to hold detailed month information
struct MonthWithDetails {
    std::uint8_t month;   // 1-12
    std::uint16_t year;   // Full year
    std::uint32_t total;
    std::array<std::uint32_t, 31> history{}; // Daily history
};

// Structure to represent a month key
struct Month {
    Month() = default;

    Month(std::uint8_t m, std::uint16_t y) : month(m), year(y) {}

    std::uint8_t month;   // 1-12
    std::uint16_t year;   // Full year

    bool operator==(const Month &other) const {
        return month == other.month && year == other.year;
    }
};

// Structure to hold month details
struct MonthDetails {
    std::uint32_t total = 0;
    std::array<std::uint32_t, 31> history{}; // Daily history
};

// Custom hash function for Month
struct MonthHash {
    size_t operator()(const Month &m) const {
        size_t res = std::hash<std::uint16_t>()(m.year);
        res ^= std::hash<std::uint8_t>()(m.month) + 0x9e3779b9 + (res << 6) + (res >> 2);
        return res;
    }
};

// Structure to hold detailed year information
struct YearWithDetails {
    std::uint16_t year;   // Full year
    std::uint32_t total;
    std::array<std::uint32_t, 12> history{}; // Monthly history
};

// Structure to hold year details
struct YearDetails {
    std::uint32_t total = 0;
    std::array<std::uint32_t, 12> history{}; // Monthly history
};

// HeatingHistory class definition
class HeatingHistory {
private:
    std::unordered_map<Day, DayDetails, DayHash> dayHistory;
    std::unordered_map<Month, MonthDetails, MonthHash> monthHistory;
    std::unordered_map<std::uint16_t, YearDetails> yearHistory;
    std::vector<RunTime> runTimes;

    // Mutex for thread safety (optional)
    // mutable std::mutex historyMutex;

public:
    HeatingHistory();

    explicit HeatingHistory(const std::vector<RunTime> &runs);

    HeatingHistory(const std::vector<RunTime> &runs, const std::vector<DayWithDetails> &days,
                   const std::vector<MonthWithDetails> &months, const std::vector<YearWithDetails> &years);

    void addRunTime(const RunTime &run, bool needUpdate = false);

    void addRunTime(const std::vector<RunTime> &runs);

    void optimizeHistory();

    void optimizeRunTimes();

    std::vector<RunTime> getRunTimes() const;

    std::vector<DayWithDetails> getDayHistory();

    std::vector<MonthWithDetails> getMonthHistory();

    std::vector<YearWithDetails> getYearHistory();

    DayDetails getDayHistory(const Day &d) const;

    MonthDetails getMonthHistory(const Month &m) const;

    YearDetails getYearHistory(const std::uint16_t &y) const;

    std::vector<DayWithDetails> get31DaysHistory() const;

    std::vector<MonthWithDetails> get12MonthsHistory() const;

    std::vector<YearWithDetails> get10YearsHistory() const;

    void addRunTime(time_t start, time_t end, const std::vector<RoomData> &roomsData);
};

#endif //ESP32_TERMOSTAT_HEATINGHISTORY_H
