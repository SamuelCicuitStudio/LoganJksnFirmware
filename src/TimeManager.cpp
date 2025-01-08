#include "TimeManager.h"

/**
 * @brief Constructor for the TimeManager class.
 * 
 * Initializes the NTPClient with parameters defined in Config.h.
 * The RTCManager pointer is initialized to handle the internal RTC.
 * 
 * @param ntpServer The NTP server to use for time synchronization (default: defined in Config.h).
 * @param timeOffset The time offset to apply to the NTP time (default: defined in Config.h).
 * @param updateInterval The interval (in milliseconds) between NTP updates (default: defined in Config.h).
 * @param RTC Pointer to an RTCManager object to handle internal RTC operations.
 */
TimeManager::TimeManager(const char* ntpServer, long timeOffset, unsigned long updateInterval, RTCManager* RTC) 
    : timeClient(ntpUDP, ntpServer, timeOffset, updateInterval) {
    this->timeOffset = timeOffset;
    this->updateInterval = updateInterval;
    this->RTC = RTC;  // Initialize RTCManager pointer
}

/**
 * @brief Initializes the NTP client and Wi-Fi connection.
 * 
 * This function starts the NTPClient and establishes a Wi-Fi connection
 * to synchronize time with an NTP server.
 */
void TimeManager::initialize() {
    // Initialize NTPClient
    timeClient.begin();
}

/**
 * @brief Updates the time by fetching the latest time from the NTP server.
 * 
 * This function fetches the current time from the NTP server and updates
 * the internal RTC by setting it to the current time in Unix timestamp format.
 */
void TimeManager::UpdateTimeFromNTP() {
    timeClient.update();  // Fetch the current time from NTP server
    RTC->setUnixTime(getUnixTime());  // Set the internal RTC with the latest Unix timestamp
}

/**
 * @brief Retrieves the current Unix time (seconds since Jan 1, 1970).
 * 
 * This function returns the current time as a Unix timestamp, which represents
 * the number of seconds since January 1, 1970 (the Unix epoch).
 * 
 * @return The current Unix timestamp.
 */
unsigned long TimeManager::getUnixTime() {
    return timeClient.getEpochTime();  // Return Unix timestamp
}

/**
 * @brief Retrieves the current formatted time as a string (HH:MM:SS).
 * 
 * This function formats the current time into a string in the format "HH:MM:SS"
 * using the NTPClient library's built-in formatting.
 * 
 * @return A string representing the formatted time (e.g., "12:34:56").
 */
String TimeManager::getFormattedTime() {
    String formattedTime = timeClient.getFormattedTime();  // Get formatted time string
    return formattedTime;
}
