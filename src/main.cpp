#include "ConfigManager.h"  // Include ConfigManager library for configuration handling
#include "RTCManager.h"     // Include RTCManager library for real-time clock management
#include "WiFiManager.h"    // Include WiFiManager library for Wi-Fi connectivity
#include "TimeManager.h"    // Include TimeManager library for time synchronization
#include "Device.h"         // Include Device library for device control

struct tm timeInfo;

// Define the system mode enumeration
enum SystemMode {
    Admin,
    Normal,
    PowerFail
};

// Global variable to store the current system mode
SystemMode SysMode = Normal;

bool isLEDFlagSet();  // Checks if the LED flag is set
void handleLEDFlagAndSleep();  // Manages LED blinking and enters deep sleep if the flag is set
void AdminSetupMode();  // Starts the Wi-Fi setup mode and waits for a connection
void connectAndUpdateTime();  // Connects to Wi-Fi and updates RTC time from NTP server
void PowerFailSafeMode();  // Manages power failure safe mode to correct RTC time
void NormalMode();  // Handles the normal mode of device operation
void setTime(struct tm* timeinfo);

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

    
    setTime(&timeInfo);// Set the time to the internal RTC (ESP32 hardware)

    handleLEDFlagAndSleep();  // Check if the LED flag is set, and blink LED if necessary

    RTC = new RTCManager(&timeInfo);  // Create an RTCManager instance


    Config->CountdownDelay(5000);  // Countdown delay of 5 seconds before user action
    if (!device->isButtonPressed()){ 
        AdminSetupMode();
        goto out;
        }  // Enter setup mode if the button is not pressed

    PowerFailSafeMode();  // Handle power failure safe mode to correct RTC time via Wi-Fi
    out:;
}

void loop() {
    if(SysMode == Normal) NormalMode();  // Handle normal mode operation in the loop
    esp_task_wdt_reset();
    delay(50); // Small delay to reduce CPU usage
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
    return Config->GetBool(LED_STATE, false);
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
        unsigned long startMillis = millis(); // Start time for LED blinking
        unsigned long blinkDuration = 120000; // 2 minutes in milliseconds
        unsigned long blinkInterval = 300;   // 300ms blink interval

        // Blink LED for 2 minutes
        while (millis() - startMillis < blinkDuration) {
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
    SysMode = Admin;
    
}
/**
 * @brief Attempts to connect to Wi-Fi and update the time from the NTP server.
 * 
 * This function tries to connect to Wi-Fi up to 10 times. If successful, it initializes and updates the RTC time 
 * from the NTP server. If the connection fails after 10 attempts, the device will restart.
 */
void connectAndUpdateTime() {
    Time = new TimeManager(NTP_SERVER, TIMEOFFSET, NTP_UPDATE_INTERVAL, RTC);  // Initialize TimeManager instance
    wifi = new WiFiManager(Config, RTC, device);  // Create a Wi-Fi Manager instance
    const int maxAttempts = 10;  // Maximum number of connection attempts
    int attempts = 0;  // Initialize attempt counter

    // Attempt to connect to Wi-Fi
    while (attempts < maxAttempts) {
        wifi->connectToWiFi();  // Attempt to connect to Wi-Fi

        // If Wi-Fi is successfully connected
        if (wifi->isStillConnected()) {
            Time->initialize();  // Initialize the time manager only if Wi-Fi is connected
            Time->UpdateTimeFromNTP();  // Update the RTC time from the NTP server
            Config->RestartSysDelay(5000);  // Restart the system after 5 seconds
        }

        attempts++;  // Increment attempt counter
        delay(1000);  // Wait for 1 second before the next attempt
    }

    // If connection fails after 10 attempts, restart the device
    Serial.println("Wi-Fi connection failed after 10 attempts. Restarting...");
    Config->RestartSysDelay(5000);  // Restart the system after 5 seconds
}

/**
 * @brief Handles the power failure safe mode logic.
 * 
 * This function checks if the wake-up cause is not a timer or if the time difference between the current RTC time 
 * and the last saved time exceeds a threshold. If either of these conditions is met, it sets the Access Point flag 
 * to reconfigure the device. It then attempts to connect to Wi-Fi, and if the connection is successful, it initializes 
 * and updates the time using the NTP server.
 */
void PowerFailSafeMode() {
        
    // Get the current RTC time and the last saved time
    long currentTime = RTC->getUnixTime();
    long lastSavedTime = Config->GetULong64(LAST_TIME_SAVED, 0);

    // Calculate the time difference and check if it's greater than the threshold
    long timeDifference = currentTime - lastSavedTime;

    // If the wake-up cause is not a timer or the time difference exceeds the threshold
    if (device->getWakeUpCause() != 0 || (timeDifference < -TIME_ERROR_THRESHOLD || timeDifference > TIME_ERROR_THRESHOLD)) {
        Config->SetAPFLag();  // Set Access Point flag if conditions are met
    };
    connectAndUpdateTime();
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
    if (currentTime >= AlarmSavedTime) {
        // If the current time is past the alarm time, update the LED state flag to true
        Config->PutBool(LED_STATE, true);  // Set the LED state to ON
        
        // Handle LED flag and sleep behavior (e.g., blink or hold the LED on before sleeping)
        handleLEDFlagAndSleep();  // This function manages LED operation and transitions to sleep mode
        
    } else {
        // If the current time is less than the alarm time, update the saved time
        Config->PutULong64(CURRENT_TIME_SAVED, RTC->getUnixTime());  // Save the current time
        Config->PutULong64(LAST_TIME_SAVED, RTC->getUnixTime());  // Save the last checked time
        
        // Enter deep sleep mode for the specified duration
        device->deepSleep(DEEPSLEEP_TIME);  // The device will enter deep sleep for a predefined amount of time
    }
}

// Set time to internal RTC memory
void setTime(struct tm* timeinfo) {
  // This sets the time using internal RTC memory
  struct timeval now = { 0 };
  now.tv_sec = mktime(timeinfo);  // Convert struct tm to seconds
  settimeofday(&now, NULL);       // Set the time
}