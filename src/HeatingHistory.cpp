#include "HeatingHistory.h"
#include <ctime>
#include <chrono>
#include <algorithm>
#include <array>
#include "SaveLoad.h"

// Helper function to get the number of days in a month
std::uint8_t getDaysInMonth(std::uint8_t month, std::uint16_t year) {
    static const std::array<std::uint8_t, 12> daysInMonth = {
            31, // January
            28, // February (will adjust for leap years)
            31, // March
            30, // April
            31, // May
            30, // June
            31, // July
            31, // August
            30, // September
            31, // October
            30, // November
            31  // December
    };
    if (month < 1 || month > 12) {
        return 0; // Invalid month
    }
    if (month == 2) {
        // Check for leap year
        if ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0) {
            return 29;
        }
    }
    return daysInMonth[month - 1];
}

// Helper function to truncate a time_point to the start of the day (midnight)
std::chrono::system_clock::time_point truncateToDay(const std::chrono::system_clock::time_point& tp) {
    time_t tt = std::chrono::system_clock::to_time_t(tp);
    tm tm_time;
    localtime_r(&tt, &tm_time); // Thread-safe conversion

    // Reset to midnight
    tm_time.tm_hour = 0;
    tm_time.tm_min = 0;
    tm_time.tm_sec = 0;

    time_t truncated_tt = mktime(&tm_time);
    return std::chrono::system_clock::from_time_t(truncated_tt);
}

// Default constructor
HeatingHistory::HeatingHistory() = default;

// Constructor with run times
HeatingHistory::HeatingHistory(const std::vector<RunTime> &runs) {
    for (const auto &run : runs) {
        addRunTime(run);
    }
    optimizeRunTimes();
    // saveHistory(); // Ensure saveHistory is implemented
}

// Constructor with run times and historical data
HeatingHistory::HeatingHistory(const std::vector<RunTime> &runs, const std::vector<DayWithDetails> &days,
                               const std::vector<MonthWithDetails> &months,
                               const std::vector<YearWithDetails> &years) {
    for (const auto &run : runs) {
        addRunTime(run);
    }
    for (const auto &dayDetail : days) {
        Day day(dayDetail.day, dayDetail.month, dayDetail.year);
        DayDetails &details = dayHistory[day];
        details.total = dayDetail.total;
        std::copy(dayDetail.history.begin(), dayDetail.history.end(), details.history.begin());
    }
    for (const auto &monthDetail : months) {
        Month month(monthDetail.month, monthDetail.year);
        MonthDetails &details = monthHistory[month];
        details.total = monthDetail.total;
        std::copy(monthDetail.history.begin(), monthDetail.history.end(), details.history.begin());
    }
    for (const auto &yearDetail : years) {
        YearDetails &details = yearHistory[yearDetail.year];
        details.total = yearDetail.total;
        std::copy(yearDetail.history.begin(), yearDetail.history.end(), details.history.begin());
    }
    optimizeHistory();
    optimizeRunTimes();
}

// Add a single run time
void HeatingHistory::addRunTime(const RunTime &run, bool needUpdate) {
    addRunTime(run.start, run.end, run.roomsData);
    if (needUpdate) {
        optimizeRunTimes();
        // saveHistory(); // Ensure saveHistory is implemented
    }
}

// Add multiple run times
void HeatingHistory::addRunTime(const std::vector<RunTime> &runs) {
    for (const auto &run : runs) {
        addRunTime(run);
    }
    optimizeRunTimes();
    // saveHistory(); // Ensure saveHistory is implemented
}

// Optimize historical data by removing outdated entries
void HeatingHistory::optimizeHistory() {
    // Current time
    auto now = std::chrono::system_clock::now();

    // Remove dayHistory entries older than 31 days
    auto dayCutoff = now - std::chrono::hours(24 * 31);
    for (auto it = dayHistory.begin(); it != dayHistory.end();) {
        // Create a time_point representing the start of the day
        tm dateTm{};
        dateTm.tm_mday = it->first.day;
        dateTm.tm_mon = it->first.month - 1; // Adjust for tm_mon (0-11)
        dateTm.tm_year = it->first.year - 1900; // Adjust for tm_year (years since 1900)
        dateTm.tm_hour = 0;
        dateTm.tm_min = 0;
        dateTm.tm_sec = 0;
        dateTm.tm_isdst = -1; // Let mktime determine DST

        time_t date_time_t = mktime(&dateTm);
        if (date_time_t == -1) {
            // Invalid date, remove entry
            it = dayHistory.erase(it);
            continue;
        }

        std::chrono::system_clock::time_point date_time_point = std::chrono::system_clock::from_time_t(date_time_t);
        if (date_time_point < dayCutoff) {
            it = dayHistory.erase(it);
        } else {
            ++it;
        }
    }

    // Remove monthHistory entries older than 12 months
    auto monthCutoff = now - std::chrono::hours(24 * 365); // Approximate 12 months
    for (auto it = monthHistory.begin(); it != monthHistory.end();) {
        // Create a time_point representing the start of the month
        tm dateTm{};
        dateTm.tm_mday = 1;
        dateTm.tm_mon = it->first.month - 1; // Adjust for tm_mon (0-11)
        dateTm.tm_year = it->first.year - 1900; // Adjust for tm_year (years since 1900)
        dateTm.tm_hour = 0;
        dateTm.tm_min = 0;
        dateTm.tm_sec = 0;
        dateTm.tm_isdst = -1; // Let mktime determine DST

        time_t date_time_t = mktime(&dateTm);
        if (date_time_t == -1) {
            // Invalid date, remove entry
            it = monthHistory.erase(it);
            continue;
        }

        std::chrono::system_clock::time_point date_time_point = std::chrono::system_clock::from_time_t(date_time_t);
        if (date_time_point < monthCutoff) {
            it = monthHistory.erase(it);
        } else {
            ++it;
        }
    }

    // Remove yearHistory entries older than 10 years
    auto yearCutoff = now - std::chrono::hours(24 * 365 * 10); // Approximate 10 years
    for (auto it = yearHistory.begin(); it != yearHistory.end();) {
        // Create a time_point representing the start of the year
        tm dateTm{};
        dateTm.tm_mday = 1;
        dateTm.tm_mon = 0; // January
        dateTm.tm_year = it->first - 1900; // Adjust for tm_year (years since 1900)
        dateTm.tm_hour = 0;
        dateTm.tm_min = 0;
        dateTm.tm_sec = 0;
        dateTm.tm_isdst = -1; // Let mktime determine DST

        time_t date_time_t = mktime(&dateTm);
        if (date_time_t == -1) {
            // Invalid date, remove entry
            it = yearHistory.erase(it);
            continue;
        }

        std::chrono::system_clock::time_point date_time_point = std::chrono::system_clock::from_time_t(date_time_t);
        if (date_time_point < yearCutoff) {
            it = yearHistory.erase(it);
        } else {
            ++it;
        }
    }
}

// Optimize run times by removing entries older than 7 days
void HeatingHistory::optimizeRunTimes() {
    // Current time
    auto now = std::chrono::system_clock::now();
    auto cutoff = now - std::chrono::hours(24 * 7);
    time_t cutoff_time_t = std::chrono::system_clock::to_time_t(cutoff);

    // Remove runTimes older than 7 days
    runTimes.erase(
            std::remove_if(runTimes.begin(), runTimes.end(),
                           [cutoff_time_t](const RunTime &run) { return run.start < cutoff_time_t; }),
            runTimes.end()
    );
}

// Retrieve all run times
std::vector<RunTime> HeatingHistory::getRunTimes() const {
    return runTimes;
}

// Retrieve all day history
std::vector<DayWithDetails> HeatingHistory::getDayHistory() {
    optimizeHistory();
    std::vector<DayWithDetails> result;
    result.reserve(dayHistory.size());
    for (const auto &entry : dayHistory) {
        DayWithDetails dayWithDetails{};
        dayWithDetails.day = entry.first.day;
        dayWithDetails.month = entry.first.month;
        dayWithDetails.year = entry.first.year;
        dayWithDetails.total = entry.second.total;
        std::copy(entry.second.history.begin(), entry.second.history.end(), dayWithDetails.history.begin());
        result.push_back(dayWithDetails);
    }
    return result;
}

// Retrieve all month history
std::vector<MonthWithDetails> HeatingHistory::getMonthHistory() {
    optimizeHistory();
    std::vector<MonthWithDetails> result;
    result.reserve(monthHistory.size());
    for (const auto &entry : monthHistory) {
        MonthWithDetails monthWithDetails{};
        monthWithDetails.month = entry.first.month;
        monthWithDetails.year = entry.first.year;
        monthWithDetails.total = entry.second.total;
        std::copy(entry.second.history.begin(), entry.second.history.end(), monthWithDetails.history.begin());
        result.push_back(monthWithDetails);
    }
    return result;
}

// Retrieve all year history
std::vector<YearWithDetails> HeatingHistory::getYearHistory() {
    optimizeHistory();
    std::vector<YearWithDetails> result;
    result.reserve(yearHistory.size());
    for (const auto &entry : yearHistory) {
        YearWithDetails yearWithDetails{};
        yearWithDetails.year = entry.first;
        yearWithDetails.total = entry.second.total;
        std::copy(entry.second.history.begin(), entry.second.history.end(), yearWithDetails.history.begin());
        result.push_back(yearWithDetails);
    }
    return result;
}

// Retrieve day history for a specific day
DayDetails HeatingHistory::getDayHistory(const Day &d) const {
    auto it = dayHistory.find(d);
    if (it != dayHistory.end()) {
        return it->second;
    } else {
        return DayDetails{};
    }
}

// Retrieve month history for a specific month
MonthDetails HeatingHistory::getMonthHistory(const Month &m) const {
    auto it = monthHistory.find(m);
    if (it != monthHistory.end()) {
        return it->second;
    } else {
        return MonthDetails{};
    }
}

// Retrieve year history for a specific year
YearDetails HeatingHistory::getYearHistory(const std::uint16_t &y) const {
    auto it = yearHistory.find(y);
    if (it != yearHistory.end()) {
        return it->second;
    } else {
        return YearDetails{};
    }
}

// Retrieve history for the last 31 days
std::vector<DayWithDetails> HeatingHistory::get31DaysHistory() const {
    std::vector<DayWithDetails> result;
    result.reserve(31);

    auto now = std::chrono::system_clock::now();
    auto today = truncateToDay(now);

    for (int i = 30; i >= 0; --i) {
        auto day_time_point = today - std::chrono::hours(24 * i);
        time_t day_time_t = std::chrono::system_clock::to_time_t(day_time_point);
        tm dayTm{};
        localtime_r(&day_time_t, &dayTm);

        Day day(dayTm.tm_mday, dayTm.tm_mon + 1, dayTm.tm_year + 1900);
        DayWithDetails dayWithDetails{};
        dayWithDetails.day = day.day;
        dayWithDetails.month = day.month;
        dayWithDetails.year = day.year;

        auto it = dayHistory.find(day);
        if (it != dayHistory.end()) {
            dayWithDetails.total = it->second.total;
            std::copy(it->second.history.begin(), it->second.history.end(), dayWithDetails.history.begin());
        }
        result.push_back(dayWithDetails);
    }
    return result;
}

// Retrieve history for the last 12 months
std::vector<MonthWithDetails> HeatingHistory::get12MonthsHistory() const {
    std::vector<MonthWithDetails> result;
    result.reserve(12);

    auto now = std::chrono::system_clock::now();
    tm nowTm{};
    time_t now_time_t = std::chrono::system_clock::to_time_t(now);
    localtime_r(&now_time_t, &nowTm);

    std::uint8_t currentMonth = nowTm.tm_mon + 1;
    std::uint16_t currentYear = nowTm.tm_year + 1900;

    for (int i = 0; i < 12; ++i) {
        int monthOffset = currentMonth - i - 1;
        int yearOffset = currentYear;
        if (monthOffset <= 0) {
            monthOffset += 12;
            yearOffset -= 1;
        }
        Month month(monthOffset, yearOffset);

        MonthWithDetails monthWithDetails{};
        monthWithDetails.month = month.month;
        monthWithDetails.year = month.year;

        auto it = monthHistory.find(month);
        if (it != monthHistory.end()) {
            monthWithDetails.total = it->second.total;
            std::copy(it->second.history.begin(), it->second.history.end(), monthWithDetails.history.begin());
        }
        result.push_back(monthWithDetails);
    }
    return result;
}

// Retrieve history for the last 10 years
std::vector<YearWithDetails> HeatingHistory::get10YearsHistory() const {
    std::vector<YearWithDetails> result;
    result.reserve(10);

    auto now = std::chrono::system_clock::now();
    tm nowTm{};
    time_t now_time_t = std::chrono::system_clock::to_time_t(now);
    localtime_r(&now_time_t, &nowTm);

    std::uint16_t currentYear = nowTm.tm_year + 1900;

    for (int i = 0; i < 10; ++i) {
        std::uint16_t year = currentYear - i;

        YearWithDetails yearWithDetails{};
        yearWithDetails.year = year;

        auto it = yearHistory.find(year);
        if (it != yearHistory.end()) {
            yearWithDetails.total = it->second.total;
            std::copy(it->second.history.begin(), it->second.history.end(), yearWithDetails.history.begin());
        }
        result.push_back(yearWithDetails);
    }
    return result;
}

// Add a run time with start, end, and room data
void HeatingHistory::addRunTime(time_t start, time_t end, const std::vector<RoomData> &roomsData) {
    if (end <= start) {
        return; // Invalid time range
    }

    // Create a RunTime object and store it
    RunTime run{start, end, roomsData};
    runTimes.push_back(run);

    // Convert to std::chrono for easier manipulation
    std::chrono::system_clock::time_point startTimePoint = std::chrono::system_clock::from_time_t(start);
    std::chrono::system_clock::time_point endTimePoint = std::chrono::system_clock::from_time_t(end);

    // Iterate through each day in the runtime
    for (auto timePoint = startTimePoint; timePoint < endTimePoint;) {
        // Truncate to the start of the current day
        std::chrono::system_clock::time_point dayStart = truncateToDay(timePoint);

        // Calculate the end of the current day
        std::chrono::system_clock::time_point nextDayStart = dayStart + std::chrono::hours(24);
        std::chrono::system_clock::time_point segmentEnd = (endTimePoint < nextDayStart) ? endTimePoint : nextDayStart;

        // Duration in seconds for this segment
        auto durationSeconds = std::chrono::duration_cast<std::chrono::seconds>(segmentEnd - timePoint).count();

        // Get the date components
        time_t current_time_t = std::chrono::system_clock::to_time_t(timePoint);
        tm currentTm{};
        localtime_r(&current_time_t, &currentTm);

        std::uint8_t day = currentTm.tm_mday;
        std::uint8_t month = currentTm.tm_mon + 1; // tm_mon is 0-11
        std::uint16_t year = currentTm.tm_year + 1900; // tm_year is years since 1900

        Day currentDay(day, month, year);

        // Update dayHistory
        DayDetails &dayDetails = dayHistory[currentDay];
        dayDetails.total += durationSeconds;

        // Update hour-wise history
        auto segmentTimePoint = timePoint;
        while (segmentTimePoint < segmentEnd) {
            time_t segmentTime_t = std::chrono::system_clock::to_time_t(segmentTimePoint);
            tm segmentTm{};
            localtime_r(&segmentTime_t, &segmentTm);
            std::uint8_t hour = segmentTm.tm_hour;

            // Calculate the end of the current hour
            std::chrono::system_clock::time_point hourStart = truncateToDay(segmentTimePoint) + std::chrono::hours(hour);
            std::chrono::system_clock::time_point nextHourStart = hourStart + std::chrono::hours(1);
            std::chrono::system_clock::time_point hourSegmentEnd = (segmentEnd < nextHourStart) ? segmentEnd : nextHourStart;

            // Duration in seconds for this hour segment
            auto hourDurationSeconds = std::chrono::duration_cast<std::chrono::seconds>(hourSegmentEnd - segmentTimePoint).count();

            // Update the history
            dayDetails.history[hour] += hourDurationSeconds;

            // Move to the next hour
            segmentTimePoint = hourSegmentEnd;
        }

        // Update monthHistory
        Month currentMonth(month, year);
        MonthDetails &monthDetails = monthHistory[currentMonth];
        monthDetails.total += durationSeconds;
        if (day <= 31) {
            monthDetails.history[day - 1] += durationSeconds; // day - 1 for 0-based index
        }

        // Update yearHistory
        YearDetails &yearDetails = yearHistory[year];
        yearDetails.total += durationSeconds;
        if (month <= 12) {
            yearDetails.history[month - 1] += durationSeconds;
        }
        timePoint = segmentEnd;
    }
}
