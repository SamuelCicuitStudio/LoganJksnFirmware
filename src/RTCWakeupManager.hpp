#ifndef RTCWAKEUPMANAGER_HPP
#define RTCWAKEUPMANAGER_HPP
#include "ConfigManager.hpp"



class RTCWakeupManager {
public:
    RTCWakeupManager();  // Constructor
    // Function to initiate deep sleep for a specified duration
    void enterDeepSleep(uint64_t sleepDuration);

    // Function to reset the system (trigger a restart)
    void resetSystem();
};

#endif // RTCWAKEUPMANAGER_HPP
