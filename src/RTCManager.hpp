// RTCManager.hpp
#ifndef RTC_MANAGER_HPP
#define RTC_MANAGER_HPP

#include <sys/time.h>
#include <ctime>
#include <string>

class RTCManager {
public:
    RTCManager();  // Constructor declaration
    // Sets the RTC time with year, month, day, hour, minute, and second
    static void setTime(int year, int month, int day, int hour, int minute, int second);

    // Gets the current time as a formatted string
    static std::string getTime();

    // Gets the current Unix timestamp
    static time_t getTimestamp();

    // Gets the current Unix timestamp as an integer
    static int getTimestampAsInt();
    void setTimeFromTimestamp(int timestamp);
    
};

#endif // RTC_MANAGER_HPP
