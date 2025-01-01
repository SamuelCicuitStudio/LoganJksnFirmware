#include "RTCWakeupManager.hpp"
#define WakeMode RF_DISABLED // Set the RF module to be disabled during sleep

RTCWakeupManager::RTCWakeupManager() {
    // Constructor can be used for any initialization if needed
}

/**
 * @brief Puts the ESP8266 into deep sleep for a specified duration.
 *
 * The device will enter deep sleep and wake up after the specified duration. 
 * Note: GPIO16 (D0) must be connected to the reset pin (RST) for wakeup.
 *
 * @param sleepDuration The sleep duration in microseconds.
 */
void RTCWakeupManager::enterDeepSleep(uint64_t sleepDuration) {
    // Set the ESP8266 to deep sleep with RF disabled for minimal power consumption
    ESP.deepSleep(sleepDuration, WakeMode);  // Enter deep sleep
    delay(100);                             // Small delay to ensure deep sleep starts properly
}

/**
 * @brief Resets the system (triggers a restart).
 */
void RTCWakeupManager::resetSystem() {
    ESP.restart();  // Reset the ESP8266, causing a restart
}
