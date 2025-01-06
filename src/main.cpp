#include "ConfigManager.h"  // Include ConfigManager library for configuration handling
#include "RTCManager.h"     // Include RTCManager library for real-time clock management
#include "WiFiManager.h"    // Include WiFiManager library for Wi-Fi connectivity
#include "TimeManager.h"    // Include TimeManager library for time synchronization
#include "Device.h"         // Include Device library for device control

struct tm timeInfo;

bool isLEDFlagSet();  // Checks if the LED flag is set
void handleLEDFlagAndSleep();  // Manages LED blinking and enters deep sleep if the flag is set
void AdminSetupMode();  // Starts the Wi-Fi setup mode and waits for a connection
void connectAndUpdateTime();  // Connects to Wi-Fi and updates RTC time from NTP server
void PowerFailSafeMode();  // Manages power failure safe mode to correct RTC time
void NormalMode();  // Handles the normal mode of device operation
void setUnixTime(unsigned long timestamp);

Preferences prefs;  // Create a Preferences object for storing configuration settings

// Initialize global pointers for configuration, RTC, and Wi-Fi management
ConfigManager *Config = nullptr;  // Configuration manager pointer
RTCManager *RTC = nullptr;        // RTC manager pointer
TimeManager *Time = nullptr;      // Time manager pointer
WiFiManager *wifi = nullptr;      // Wi-Fi manager pointer
Device *device = nullptr;         // Device pointer

void setup() {
    Serial.begin(SERIAL_BAUD_RATE);  // Start serial communication
    prefs.begin(CONFIG_PARTITION, false);  // Open Preferences in read-write mode
    Config = new ConfigManager(&prefs);  // Initialize ConfigManager instance with Preferences
    Config->begin();  // Initialize the ConfigManager

    device = new Device();  // Create a Device instance
    device->begin();  // Initialize the Device
    
    // Set the system time using the alarm time (Unix timestamp)
    setUnixTime(Config->GetULong64(CURRENT_TIME_SAVED, 0));

    handleLEDFlagAndSleep();  // Check if the LED flag is set, and blink LED if necessary

    RTC = new RTCManager(&timeInfo);  // Create an RTCManager instance

    Config->CountdownDelay(5000);  // Countdown delay of 5 seconds before user action
    if (device->isButtonPressed() != false) { 
        Serial.println("Entering Admin Mode");
        AdminSetupMode();
        goto out;
    }  // Enter setup mode if the button is not pressed

    PowerFailSafeMode();  // Handle power failure safe mode to correct RTC time via Wi-Fi
out:;
}

void loop() {
    esp_task_wdt_reset();  // Reset the watchdog timer to prevent a system reset
    delay(50);  // Small delay to reduce CPU usage
}

/**
 * @brief Retrieves the current state of the LED from the configuration.
 *
 * This function checks the configuration for the LED state and returns the 
 * value. If no value is set, it returns `false` by default.
 *
 * @return `true` if the LED state is enabled, `false` otherwise.
 */
bool isLEDFlagSet() {
    return Config->GetBool(LED_STATE, false);  // Retrieve the LED state from the configuration
}

/**
 * @brief Checks if the LED flag is set and handles LED blinking and deep sleep.
 *
 * This function checks if the LED flag is set, and if so, it blinks the LED
 * for 2 minutes. After the blinking duration, the device enters deep sleep 
 * mode for 5 minutes.
 */
void handleLEDFlagAndSleep() {
    // Check if the LED flag is set
    if (isLEDFlagSet()) {
        unsigned long startMillis = millis();  // Start time for LED blinking
        unsigned long blinkDuration = 120000;  // 2 minutes in milliseconds
        unsigned long blinkInterval = 300;    // 300ms blink interval

        // Blink LED for 2 minutes
        while (millis() - startMillis < blinkDuration) {
            if (device->isButtonPressed() != false) return;
            device->blinkLED(blinkInterval);  // Blink LED at the specified interval
        }

        // After 2 minutes of blinking, enter deep sleep for 5 minutes
        device->deepSleep(300000);  // 5 minutes in milliseconds
    }
}

/**
 * @brief Initiates the Wi-Fi setup mode and waits for a connection.
 * 
 * This function attempts to connect to Wi-Fi using the `wifi->begin()` method. 
 * It continuously checks if the connection is established. If Wi-Fi is connected, 
 * the function exits the loop. If the connection is not established within a timeout 
 * period of 2 minutes, the system is restarted.
 * 
 * @note The function will restart the system if the connection attempt times out.
 */
void AdminSetupMode() {
    wifi = new WiFiManager(Config, RTC, device);  // Create a Wi-Fi Manager instance
    // Try to start Wi-Fi
    wifi->begin();   
}

/**
 * @brief Attempts to connect to Wi-Fi and update the time from the NTP server.
 * 
 * This function tries to connect to Wi-Fi up to 10 times. If successful, it initializes and updates the RTC time 
 * from the NTP server. If the connection fails after 10 attempts, the device will restart.
 */
void connectAndUpdateTime() {
    Time = new TimeManager(NTP_SERVER, TIMEOFFSET, NTP_UPDATE_INTERVAL, RTC); // Initialize TimeManager instance
    wifi = new WiFiManager(Config, RTC, device);  // Create a Wi-Fi Manager instance
    const int maxAttempts = 10;  // Maximum number of connection attempts
    int attempts = 0;            // Initialize attempt counter
    // Attempt to connect to Wi-Fi
    while (attempts < maxAttempts) {
        wifi->connectToWiFi();  // Attempt to connect to Wi-Fi
        // If Wi-Fi is successfully connected
        if (wifi->isStillConnected()) {
            Serial.println("Initialize the time manager only if Wi-Fi is connected");
            Time->initialize();  // Initialize the time manager only if Wi-Fi is connected

            Serial.println("Update the RTC time from the NTP server");
            if (Time->UpdateTimeFromNTP()) {  // Update the RTC time from the NTP server
                Serial.println("Updating the Strored Timestamp ");
                Config->PutULong64(CURRENT_TIME_SAVED, RTC->getUnixTime());  // Save the current time
                Config->PutULong64(LAST_TIME_SAVED, RTC->getUnixTime());  // Save the last checked time
                Serial.println("Start Normal Mode");
                NormalMode();  // Enter normal mode
                return;  // Exit the function after successful setup
            } else {
                Serial.println("Failed to update time from NTP. Retrying...");
                attempts++;      // Increment attempt counter
                delay(1000);     // Wait for 1 second before the next attempt
                continue;        // Retry if time update fails
            }
        }

        attempts++;  // Increment attempt counter
        delay(1000); // Wait for 1 second before the next attempt
    }

    // If connection fails after max attempts, restart the device
    Serial.println("Wi-Fi connection failed after 10 attempts. Restarting...");
    Config->RestartSysDelay(5000);  // Restart the system after 5 seconds
}

/**
 * @brief Handles the power failure safe mode logic.
 * 
 * This function checks if the wake-up cause is not a timer or if the time difference between the current RTC time 
 * and the last saved time exceeds a threshold. If either condition is met, it sets the system mode to Normal. 
 * If the conditions do not meet, it sets the system mode to PowerFail and attempts to connect to Wi-Fi and update the time.
 */
void PowerFailSafeMode() {
    Serial.println("Entering Power safe Mode");
    
    // If the wake-up cause is not a timer or if the time difference exceeds the threshold
    if (device->getWakeUpCause() == 0) {
        // Set the system mode to Normal if the condition is met
        Serial.println("Wakeup by timer. Entering Normal mode.");
        RTC->getUnixTime();  // Retrieve current Unix time
        Serial.println("Adding 1 hour.");
        RTC->setUnixTime(RTC->getUnixTime() + (DEEPSLEEP_TIME / 1000));  // Adjust time
        Config->PutULong64(CURRENT_TIME_SAVED, RTC->getUnixTime());  // Save the current time
        Config->PutULong64(LAST_TIME_SAVED, RTC->getUnixTime());  // Save the last checked time
        NormalMode();  // Go to normal mode
    } else {
        // Set the system mode to PowerFail and connect to Wi-Fi
        Serial.println("Fixing Time");
        connectAndUpdateTime();  // Attempt to connect to Wi-Fi and update time
    }
}

/**
 * @brief Handles normal mode operation for the device.
 *
 * This function checks the current time and compares it with the saved alarm time.
 * If the current time is greater than or equal to the alarm time, it sets the LED state
 * and calls a function to handle LED behavior and sleep. If the current time is less than
 * the alarm time, the function updates the saved time and enters deep sleep.
 */
void NormalMode() {
    // Get the current time from the RTC (Real-Time Clock) and the last saved alarm time from the configuration
    long currentTime = RTC->getUnixTime();  // Get the current Unix time (seconds since 1970)
    long AlarmSavedTime = Config->GetULong64(ALERT_TIMESTAMP_SAVED, 0);  // Retrieve the last saved alarm time
    
    // Check if the current time is greater than or equal to the alarm saved time
    Serial.println("Check if the current time");
    if (currentTime >= AlarmSavedTime) {
        // If the current time is past the alarm time, update the LED state flag to true
        Serial.println("Updating LED flag.");
        Config->PutBool(LED_STATE, true);
        handleLEDFlagAndSleep();  // Handle LED blinking and sleep
    } else {
        // If the current time is less than the alarm time, update the alarm time and go to deep sleep
        Serial.println("Update time and sleep");
        Config->PutULong64(ALERT_TIMESTAMP_SAVED, currentTime);  // Update the alarm time
        RTC->setUnixTime(currentTime + (DEEPSLEEP_TIME / 1000));  // Set the RTC time
        Config->RestartSysDelay(5000);  // Restart the system after 5 seconds
    }
}

/**
 * @brief Sets the Unix time on the RTC module using a Unix timestamp.
 *
 * @param timestamp Unix timestamp to set on the RTC module.
 */
void setUnixTime(unsigned long timestamp) {
    RTC->setUnixTime(timestamp);  // Set the RTC Unix time
}
