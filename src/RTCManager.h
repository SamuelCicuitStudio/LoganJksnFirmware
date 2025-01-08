#ifndef RTCMANAGER_H
#define RTCMANAGER_H


#include "ConfigManager.h"

class RTCManager {
public:
    RTCManager();  // Constructor
    void setUnixTime(unsigned long timestamp);  // Set RTC time using Unix timestamp
    unsigned long getUnixTime();  // Get current Unix timestamp
    String getFormattedTime();  // Get current time as a formatted string (HH:MM:SS)
    String getFormattedDate();  // Get current date as a formatted string (YYYY-MM-DD)

private:
    struct tm timeinfo;  // Struct to hold time information
};

#endif  // RTCMANAGER_H