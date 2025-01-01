// RTCManager.cpp
#include "RTCManager.hpp"

/**
 * @brief Constructs an RTCManager object.
 *
 * This constructor is used to initialize the RTCManager object.
 * Currently, it does not perform any additional initialization.
 */
RTCManager::RTCManager() {
    // Constructor body (empty or could include initialization logic)
}
/**
 * @brief Sets the RTC time using a Unix timestamp provided as an integer.
 *
 * This function initializes the ESP8266's internal RTC with the specified Unix timestamp.
 * @param timestamp The Unix timestamp (seconds since epoch) as an integer.
 */
void RTCManager::setTimeFromTimestamp(int timestamp) {
    struct timeval now = { .tv_sec = static_cast<time_t>(timestamp), .tv_usec = 0 };
    settimeofday(&now, NULL); // Set the RTC time
}
/**
 * @brief Sets the RTC time.
 *
 * This function initializes the ESP32's internal RTC with the specified date and time.
 * @param year The year (e.g., 2025).
 * @param month The month (1-12).
 * @param day The day of the month (1-31).
 * @param hour The hour (0-23).
 * @param minute The minute (0-59).
 * @param second The second (0-59).
 */
void RTCManager::setTime(int year, int month, int day, int hour, int minute, int second) {
    struct tm timeinfo;
    timeinfo.tm_year = year - 1900; // tm_year is year since 1900
    timeinfo.tm_mon = month - 1;   // tm_mon is 0-based (0 = January)
    timeinfo.tm_mday = day;
    timeinfo.tm_hour = hour;
    timeinfo.tm_min = minute;
    timeinfo.tm_sec = second;
    timeinfo.tm_isdst = -1;        // No daylight saving time

    time_t t = mktime(&timeinfo); // Convert to time_t (seconds since epoch)

    struct timeval now = { .tv_sec = t, .tv_usec = 0 };
    settimeofday(&now, NULL);     // Set the RTC time
}

/**
 * @brief Retrieves the current time as a formatted string.
 *
 * This function fetches the current time from the RTC and formats it as a string in the format "YYYY-MM-DD HH:MM:SS".
 * @return A string representation of the current time.
 */
std::string RTCManager::getTime() {
    struct timeval tv;
    gettimeofday(&tv, NULL);

    time_t now = tv.tv_sec;
    struct tm *timeinfo = localtime(&now); // Convert to local time

    char buffer[20];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);

    return std::string(buffer);
}

/**
 * @brief Retrieves the current Unix timestamp.
 *
 * This function fetches the current time from the RTC and returns it as a Unix timestamp (seconds since epoch).
 * @return The current Unix timestamp as a time_t.
 */
time_t RTCManager::getTimestamp() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec; // Return the Unix timestamp
}

/**
 * @brief Retrieves the current Unix timestamp as an integer.
 *
 * This function fetches the current time from the RTC and returns it as an integer.
 * @return The current Unix timestamp as an integer.
 */
int RTCManager::getTimestampAsInt() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return static_cast<int>(tv.tv_sec); // Return the Unix timestamp as an integer
}
