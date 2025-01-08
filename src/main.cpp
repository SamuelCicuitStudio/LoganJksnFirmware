#include "ConfigManager.h"  // Include ConfigManager library for configuration handling
#include "RTCManager.h"     // Include RTCManager library for real-time clock management
#include "WiFiManager.h"    // Include WiFiManager library for Wi-Fi connectivity
#include "TimeManager.h"    // Include TimeManager library for time synchronization
#include "Device.h"         // Include Device library for device control
#include <ArduinoJson.h>  // Include the ArduinoJson library

bool isLEDFlagSet();  // Checks if the LED flag is set
void handleLEDFlagAndSleep();  // Manages LED blinking and enters deep sleep if the flag is set
void AdminSetupMode();  // Starts the Wi-Fi setup mode and waits for a connection
void connectAndUpdateTime();  // Connects to Wi-Fi and updates RTC time from NTP server
void PowerFailSafeMode();  // Manages power failure safe mode to correct RTC time
void NormalMode();  // Handles the normal mode of device operation
void setFromSerial() ;// settime from serial

Preferences prefs;  // Create a Preferences object for storing configuration settings

// Initialize global pointers for configuration, RTC, and Wi-Fi management
ConfigManager *Config = nullptr;  // Configuration manager pointer
RTCManager *RTC = nullptr;        // RTC manager pointer
TimeManager *Time = nullptr;      // Time manager pointer
WiFiManager *wifi = nullptr;      // Wi-Fi manager pointer
Device *device = nullptr;         // Device pointer

void setup() {
    Serial.begin(SERIAL_BAUD_RATE);  // Start serial communication
    Config = new ConfigManager(&prefs);  // Initialize ConfigManager instance with Preferences
    Config->begin();  // Initialize the ConfigManager

    device = new Device();  // Create a Device instance
    device->begin();  // Initialize the Device

    
    handleLEDFlagAndSleep();  // Check if the LED flag is set, and blink LED if necessary

    RTC = new RTCManager();  // Create an RTCManager instance

    Time = new TimeManager(NTP_SERVER, TIMEOFFSET, NTP_UPDATE_INTERVAL, RTC);  // Initialize TimeManager instance

    wifi = new WiFiManager(Config, RTC, device);  // Create a Wi-Fi Manager instance
    
    if (!device->isProgButtonPressed()){
        while(true){
            setFromSerial();
        }
    }

    Config->CountdownDelay(5000);  // Countdown delay of 5 seconds before user action
    if (!device->isButtonPressed()) AdminSetupMode();  // Enter setup mode if the button is not pressed

    PowerFailSafeMode();  // Handle power failure safe mode to correct RTC time via Wi-Fi
}

void loop() {
    NormalMode();  // Handle normal mode operation in the loop
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
    unsigned long startMillis = millis();  // Store the current time

    while (true) {
        // Try to start Wi-Fi
        wifi->begin();

        // Check if Wi-Fi is connected
        if (WiFi.isConnected()) {
            Serial.println("Wi-Fi connected!");
            break;  // Exit the loop if Wi-Fi is connected
        }

        // Check if the timeout period has elapsed (2 minutes)
        if (millis() - startMillis >= WIFI_TIMEOUT) {
            Serial.println("Wi-Fi connection timed out, restarting...");
            Config->RestartSysDelay(3000);  // Restart after 2 minutes
        }

        delay(100);  // Small delay to avoid blocking the loop completely
    }
}
/**
 * @brief Attempts to connect to Wi-Fi and update the time from the NTP server.
 * 
 * This function tries to connect to Wi-Fi up to 10 times. If successful, it initializes and updates the RTC time 
 * from the NTP server. If the connection fails after 10 attempts, the device will restart.
 */
void connectAndUpdateTime() {
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
    long AlarmSavedTime = Config->GetULong64(ALERT_TIME_SAVED, 0);  // Retrieve the last saved alarm time
    
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

/**
 * @brief Sets parameters from received serial data.
 *
 * This function reads incoming JSON data from the serial port, parses it, and extracts
 * date and time information. It then constructs a datetime string in the format 
 * "YYYY-MM-DD HH:MM:SS" and converts it into a Unix timestamp. If the conversion is 
 * successful, the timestamp is saved into the preferences partition for future use.
 *
 * The expected JSON format for the incoming data is:
 * {
 *   "time": "HH:MM",  // Time in 24-hour format (e.g., "12:30")
 *   "day": "DD",      // Day in two digits (e.g., "08")
 *   "month": "MM",    // Month in two digits (e.g., "01")
 *   "year": "YYYY"    // Year in four digits (e.g., "2025")
 * }
 *
 * @note The timestamp is saved in the ESP32's preferences partition using the 
 *       `Config->PutULong64(ALERT_TIME_SAVED, alarmTimeUnix)` function.
 *
 * @see ArduinoJson library for JSON parsing
 * @see `strptime` and `mktime` functions for date/time parsing and conversion
 * 
 * @return void
 */
void setFromSerial() {
  if (Serial.available() > 0) {
    String jsonData = Serial.readStringUntil('\n'); // Read the incoming JSON data

    // Parse the JSON data
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, jsonData);

    if (error) {
      Serial.print("Error parsing JSON: ");
      Serial.println(error.f_str());
      return;
    }

    // Extract date and time values from the JSON
    const char* time = doc["time"];  // Time in format HH:MM
    const char* day = doc["day"];    // Day in format DD
    const char* month = doc["month"];  // Month in format MM
    const char* year = doc["year"];  // Year in format YYYY

    // Construct the datetime string in format YYYY-MM-DD HH:MM:SS
    String dateTimeString = String(year) + "-" + String(month) + "-" + String(day) + " " + String(time) + ":00";

    // Print the datetime string for debugging
    Serial.print("Datetime String: ");
    Serial.println(dateTimeString);

    // Convert the datetime string to Unix timestamp
    struct tm timeStruct;
    strptime(dateTimeString.c_str(), "%Y-%m-%d %H:%M:%S", &timeStruct);  // Parse datetime
    time_t alarmTimeUnix = mktime(&timeStruct);  // Convert to Unix timestamp

    // Check if the conversion was successful
    if (alarmTimeUnix == -1) {
      Serial.println("Error: Unable to convert to Unix timestamp.");
    } else {
      // Print the Unix timestamp for debugging
      Serial.print("Unix Timestamp: ");
      Serial.println(alarmTimeUnix);

      // Save the timestamp to preferences
      Config->PutULong64(ALERT_TIME_SAVED, alarmTimeUnix);
    }
  }
}
