#include "RTCManager.h"

RTCManager::RTCManager() {
    // Constructor is empty, initialization is done in initialize()
}

// Set the internal RTC time using a Unix timestamp (seconds since Jan 1, 1970)
void RTCManager::setUnixTime(unsigned long timestamp) {
    // Create a timeval struct and set the seconds value
    struct timeval tv;
    tv.tv_sec = timestamp;  // Set the seconds since the Unix epoch
    tv.tv_usec = 0;  // No microseconds

    // Set the system time from timeval structure
    settimeofday(&tv, nullptr);
}

// Get the current Unix timestamp (seconds since Jan 1, 1970)
unsigned long RTCManager::getUnixTime() {
    // Get current Unix timestamp from the internal RTC
    time_t now;
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
        // Convert time struct to Unix timestamp
        now = mktime(&timeinfo);
        return now;
    }
    return 0;  // Return 0 if the time couldn't be fetched
}

// Get the current time in formatted string (HH:MM:SS)
String RTCManager::getFormattedTime() {
    if (getLocalTime(&timeinfo)) {
        char timeString[9];  // Buffer for HH:MM:SS format
        sprintf(timeString, "%02d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
        return String(timeString);
    }
    return "00:00:00";  // Default if time can't be fetched
}

// Get the current date in formatted string (YYYY-MM-DD)
String RTCManager::getFormattedDate() {
    if (getLocalTime(&timeinfo)) {
        char dateString[11];  // Buffer for YYYY-MM-DD format
        sprintf(dateString, "%04d-%02d-%02d", timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday);
        return String(dateString);
    }
    return "0000-00-00";  // Default if date can't be fetched
}
