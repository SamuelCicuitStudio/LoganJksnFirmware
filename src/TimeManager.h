#ifndef TIMEMANAGER_H
#define TIMEMANAGER_H


#include "ConfigManager.h"
#include "RTCManager.h"

class TimeManager {
public:
    TimeManager(const char* ntpServer = NTP_SERVER, long timeOffset = TIMEOFFSET, unsigned long updateInterval = NTP_UPDATE_INTERVAL,RTCManager* RTC = nullptr);

    void initialize();          // Initialize Wi-Fi and NTP client
    bool UpdateTimeFromNTP();          // Update time from NTP server
    unsigned long getUnixTime();  // Get current time in Unix timestamp format (seconds since 1970)
    String getFormattedTime();   // Get formatted time as a string (e.g., HH:MM:SS)

private:

    WiFiUDP ntpUDP;
    NTPClient timeClient;
    long timeOffset;
    unsigned long updateInterval;
    RTCManager* RTC;
};

#endif  // TIMEMANAGER_H
