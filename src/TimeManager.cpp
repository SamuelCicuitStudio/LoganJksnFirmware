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
 * It also prints the fetched time in a human-readable format.
 * 
 * @return True if the time was successfully fetched and updated; false otherwise.
 */
bool TimeManager::UpdateTimeFromNTP() {
    esp_task_wdt_reset(); // Reset the watchdog timer to prevent a system reset
    
    Serial.println("Fetching time from NTP server...");
    
    // Update the time from the NTP server
    if (!timeClient.update()) {
        if (DEBUGMODE)Serial.println("Failed to fetch time from NTP server.");
        return false; // Return false if the NTP update fails
    }
    
    // Get the updated Unix time
    long ntpTime = getUnixTime();
    
    // Validate the NTP time (e.g., ensure it's a reasonable value)
    if (ntpTime < 946684800) { // Unix time for 2000-01-01 00:00:00
        if (DEBUGMODE)Serial.println("Invalid time fetched from NTP server.");
        return false; // Return false if the time is invalid
    }
    
    if (DEBUGMODE)Serial.print("Time fetched from NTP (Unix): ");
    if (DEBUGMODE)Serial.println(ntpTime);
    
    // Convert Unix time to human-readable format
    time_t rawTime = ntpTime;               // Convert to time_t
    struct tm *timeInfo = gmtime(&rawTime); // Convert to UTC time structure

    if (DEBUGMODE)Serial.println("################################");
    if (DEBUGMODE)Serial.print("Time fetched from NTP (Human-readable UTC): ");
    if (DEBUGMODE)Serial.printf("%04d-%02d-%02d %02d:%02d:%02d\n", 
                  timeInfo->tm_year + 1900, 
                  timeInfo->tm_mon + 1, 
                  timeInfo->tm_mday, 
                  timeInfo->tm_hour, 
                  timeInfo->tm_min, 
                  timeInfo->tm_sec);
    if (DEBUGMODE)Serial.println("################################");

    // Update the RTC with the fetched time
    if (DEBUGMODE)Serial.println("Updating RTC with the fetched time...");
    RTC->setUnixTime(ntpTime);
    if (DEBUGMODE)Serial.println("RTC successfully updated.");
    
    return true; // Return true if the time was successfully fetched and updated
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
