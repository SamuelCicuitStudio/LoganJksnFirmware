#include "ConfigManager.hpp"
#include "RTCManager.hpp"
#include "RTCWakeupManager.hpp"
#include "wifiManager.hpp"

// Initialize global pointers for configuration, RTC, and Wi-Fi management
ConfigManager *Device = nullptr;
RTCManager *RTC = nullptr;
RTCWakeupManager *RTCwake = nullptr;
WifiManager *wifi = nullptr;

ESP8266WebServer server(80); // Server port

// Flag to indicate whether the device is in Access Point (AP) mode
bool APMode = false;

void setup() {
    // Start serial communication
    Serial.begin(SERIAL_BAUD_RATE);

    // Initialize ConfigManager and set up necessary parameters
    Device = new ConfigManager();
    Device->begin();

    // Initialize RTC and Wi-Fi Manager objects
    RTC = new RTCManager();
    RTCwake = new RTCWakeupManager();
    wifi = new WifiManager(Device, &server,RTCwake);

    // Check if it's the second time the device is powered on
    if (Device->isSecondTime()) {
        goto next;
    }

    // Check if it's the first time the device is powered on
    if (!Device->isFirstTime()) {
        APMode = true; // Set APMode to true for first-time configuration
        wifi->begin(); // Start the Wi-Fi in AP mode for configuration, will set the first-time flag at the end of execution
    }

    // Check if it's the first time and second time flags are not set
    if (Device->isFirstTime() && !Device->isSecondTime()) {
        APMode = true; // Set APMode to true for configuration
        // Retrieve the current Unix timestamp from the Wi-Fi NTP server
        int timestamp = wifi->getUnixTimestampFromWiFi();
        if (timestamp != 0) {
            RTC->setTimeFromTimestamp(timestamp); // Set RTC time from retrieved timestamp
        } else {
            RTCwake->enterDeepSleep(2000000); // Enter deep sleep mode for 2 seconds to simulate power down
        }

        Device->setSecondTimeFlag(); // Set the second-time flag
        RTCwake->enterDeepSleep(2000000); // Enter deep sleep mode for 2 seconds to simulate power down
    }

next:
    delay(1000); // Delay to ensure everything is initialized
    // If the button is pressed, enable Access Point (AP) mode
    if (Device->isButtonPressed()) {
        APMode = true; // Set APMode to true to start Wi-Fi configuration
        wifi->begin(); // Start the Wi-Fi in AP mode
    }
}

void loop() {
    // If not in AP mode, proceed with normal device operations
    if (!APMode) {
        // Check if the device has been disconnected from power (battery removed)
        if (RTC->getTimestampAsInt() - Device->readUnixTimestamp(LAST_STORED_TIME_START_ADD) > 3700) {
            // Store the current timestamp if the device has been powered off for more than 1 hour
            Device->storeUnixTimestamp(LAST_STORED_TIME_START_ADD, RTC->getTimestampAsInt());
            Device->ExpiredCheck(RTC->getTimestampAsInt()); // Check if the stored timestamp is expired
            RTCwake->enterDeepSleep(DEVICE_UPDATE_CHECK_INTERVAL); // Enter deep sleep mode for 1 hour
        } else {
            // Retrieve the current Unix timestamp from the Wi-Fi NTP server
            int timestamp = wifi->getUnixTimestampFromWiFi();
            if (timestamp != 0) {
                RTC->setTimeFromTimestamp(timestamp); // Set RTC time from retrieved timestamp
                RTCwake->enterDeepSleep(2000000); // Enter deep sleep mode for 2 seconds to simulate power down
            }
        }
    } else {
        // If in AP mode, handle Wi-Fi client requests (web server interactions)
        wifi->handleClient();
    }
}
