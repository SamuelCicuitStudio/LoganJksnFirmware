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
void setFromSerial();

Preferences prefs;  // Create a Preferences object for storing configuration settings

// Initialize global pointers for configuration, RTC, and Wi-Fi management
ConfigManager *Config = nullptr;  // Configuration manager pointer
RTCManager *RTC = nullptr;        // RTC manager pointer
TimeManager *Time = nullptr;      // Time manager pointer
WiFiManager *wifi = nullptr;      // Wi-Fi manager pointer
Device *device = nullptr;         // Device pointer

void setup() {
    // Start serial communication
    Serial.begin(SERIAL_BAUD_RATE);  
    
    // Open Preferences in read-write mode
    prefs.begin(CONFIG_PARTITION, false);  
    
    // Initialize ConfigManager instance with Preferences
    Config = new ConfigManager(&prefs);  
    Config->begin();  // Initialize the ConfigManager
    
    // Create a Device instance and initialize it
    device = new Device();  
    device->begin();  // Initialize the Device
    
    // Set the system time using the alarm time (Unix timestamp)
    setUnixTime(Config->GetULong64(CURRENT_TIME_SAVED, 0));  
    
    // Check if the LED flag is set, and blink LED if necessary
    handleLEDFlagAndSleep();  
    
    // Create an RTCManager instance
    RTC = new RTCManager(&timeInfo);  

// Countdown delay of 1.2 seconds before user action
Config->CountdownDelay(1200);  

    // Check if the program button is pressed
    if (!device->isProgButtonPressed()) {
        if (DEBUGMODE) {
            Serial.println("Serial Prog Mode");
        }
        
        // Blink the LED 4 times with a loop to avoid repetitive code
        for (int i = 0; i < 7; i++) {
            device->blinkLED(100);
        }

        // Enter serial mode and continuously process data
        while (true) {
            setFromSerial();
        }
    }

    // Check if the user button is pressed
    if (device->isButtonPressed()) {
        if (DEBUGMODE) {
            Serial.println("Entering Admin Mode");
        };
        // Blink the LED 2 times with a loop to avoid repetitive code
        for (int i = 0; i < 4; i++) {
            device->blinkLED(100);
        };
        // Enter setup mode if the button is pressed
        AdminSetupMode();
        goto out;  // Exit to the out label if the button is pressed
    };

    // Handle power failure safe mode to correct RTC time via Wi-Fi
    PowerFailSafeMode();  

out:;
}

void loop() {
    // Reset the watchdog timer to prevent system reset
    esp_task_wdt_reset();
    
    // Small delay to reduce CPU usage
    delay(50);  
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
            if (device->isButtonPressed()!= false) return;
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
    wifi->begin();// Try to start Wi-Fi   
}
/**
 * @brief Attempts to connect to Wi-Fi and update the time from the NTP server.
 * 
 * This function tries to connect to Wi-Fi up to 10 times. If successful, it initializes and updates the RTC time 
 * from the NTP server. If the connection fails after 10 attempts, the device will restart.
 */
void connectAndUpdateTime() {
    // Initialize TimeManager instance
    Time = new TimeManager(NTP_SERVER, TIMEOFFSET, NTP_UPDATE_INTERVAL, RTC); 
    
    // Create a Wi-Fi Manager instance
    wifi = new WiFiManager(Config, RTC, device);  
    
    const int maxAttempts = 10; // Maximum number of connection attempts
    int attempts = 0;          // Initialize attempt counter

    // Attempt to connect to Wi-Fi
    while (attempts < maxAttempts) {
        wifi->connectToWiFi(); // Attempt to connect to Wi-Fi

        // If Wi-Fi is successfully connected
        if (wifi->isStillConnected()) {
            if (DEBUGMODE)Serial.println("Initialize the time manager only if Wi-Fi is connected");
            Time->initialize(); // Initialize the time manager only if Wi-Fi is connected

            if (DEBUGMODE)Serial.println("Update the RTC time from the NTP server");
            if (Time->UpdateTimeFromNTP()) { // Update the RTC time from the NTP server
                if (DEBUGMODE)Serial.println("Start Normal Mode");
                int unix = RTC->getUnixTime();
                Config->PutULong64(CURRENT_TIME_SAVED, unix);
                Config->PutULong64(LAST_TIME_SAVED, unix);
                NormalMode(); // Make normal mode
                return;       // Exit the function after successful setup
            } else {
                if (DEBUGMODE)Serial.println("Failed to update time from NTP. Retrying...");
                attempts++;      // Increment attempt counter
                delay(1000);     // Wait for 1 second before the next attempt
                continue;     // Retry if time update fails
            }
        }

        attempts++;      // Increment attempt counter
        delay(1000);     // Wait for 1 second before the next attempt
    }

    // If connection fails after max attempts, restart the device
    if (DEBUGMODE)Serial.println("Wi-Fi connection failed after 10 attempts. Restarting...");
    Config->RestartSysDelay(5000); // Restart the system after 5 seconds
}


/**
 * @brief Handles the power failure safe mode logic.
 * 
 * This function checks if the wake-up cause is not a timer or if the time difference between the current RTC time 
 * and the last saved time exceeds a threshold. If either condition is met, it sets the system mode to Normal. 
 * If the conditions do not meet, it sets the system mode to PowerFail and attempts to connect to Wi-Fi and update the time.
 */
void PowerFailSafeMode() {
    if (DEBUGMODE)Serial.println("Entering Power safe Mode");
    // If the wake-up cause is not a timer or if the time difference exceeds the threshold
    if (device->getWakeUpCause() == 0) {
        // Set the system mode to Normal if the condition is met
        if (DEBUGMODE)Serial.println(" Wakeup by timer Entering Normal mode");
        RTC->getUnixTime();
        
        RTC->setUnixTime(RTC->getUnixTime() + (DEEPSLEEP_TIME/1000));
        if (DEBUGMODE)Serial.print("#############################################################");
        if (DEBUGMODE)Serial.print("Adding: ");
        if (DEBUGMODE)Serial.print((DEEPSLEEP_TIME/1000));
        if (DEBUGMODE)Serial.println(" seconds.");
        if (DEBUGMODE)Serial.print("#############################################################");
        Config->PutULong64(CURRENT_TIME_SAVED, RTC->getUnixTime());  // Save the current time
        Config->PutULong64(LAST_TIME_SAVED, RTC->getUnixTime());  // Save the last checked time
        NormalMode();// go normal mode.

    } else {
        // Set the system mode to PowerFail and connect to Wi-Fi
        if (DEBUGMODE)Serial.println("Fixing Time");
        connectAndUpdateTime();
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
    if (DEBUGMODE)Serial.println("Check if the current time");
    if (currentTime >= AlarmSavedTime) {
        // If the current time is past the alarm time, update the LED state flag to true
        if (DEBUGMODE)Serial.println("Set Led FLag");
        Config->PutBool(LED_STATE, true);  // Set the LED state to ON
        
        // Handle LED flag and sleep behavior (e.g., blink or hold the LED on before sleeping)
        if (DEBUGMODE)Serial.println("Handle LED flag and sleep behavior");
        handleLEDFlagAndSleep();  // This function manages LED operation and transitions to sleep mode
        
    } else {
        // If the current time is less than the alarm time, update the saved time
        if (DEBUGMODE)Serial.println("update the saved time");
        Config->PutULong64(CURRENT_TIME_SAVED, RTC->getUnixTime());  // Save the current time
        Config->PutULong64(LAST_TIME_SAVED, RTC->getUnixTime());  // Save the last checked time
        
        // Enter deep sleep mode for the specified duration
        device->deepSleep(DEEPSLEEP_TIME);  // The device will enter deep sleep for a predefined amount of time
    }
}

/**
 * @brief Sets the system time to a specified Unix timestamp.
 * 
 * This function sets the system time to the provided Unix timestamp. The timestamp is in seconds since the Unix epoch 
 * (January 1, 1970). The function also resets the system watchdog timer to prevent a system reset while setting the time.
 * 
 * @param timestamp The Unix timestamp (seconds since epoch) to set the system time to.
 * 
 * @note The microsecond part is set to 0 as the `tv_usec` field is not used in this case.
 * 
 * @warning Ensure that the timestamp is valid and within an appropriate range for your application.
 */
void setUnixTime(unsigned long timestamp) { 
    struct timeval tv;        ///< Struct to hold the time value
    tv.tv_sec = timestamp;    ///< Set seconds since the Unix epoch
    tv.tv_usec = 0;           ///< No microseconds
    esp_task_wdt_reset();     ///< Reset the watchdog timer to prevent a system reset
    settimeofday(&tv, nullptr); ///< Set system time
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
    device->blinkLED(100);
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
    String alarmDate = doc["alarmDate"].as<String>();  // Format: YYYY-MM-DD
    String alarmTime = doc["alarmTime"].as<String>();  // Format: HH:MM

    if (alarmDate.isEmpty() || alarmTime.isEmpty()) {
      Serial.println("Error: Missing alarmDate or alarmTime");
      return;
    }

    // Debug output
    Serial.println("################################");
    Serial.println("Alarm Time Set by USER");
    Serial.println("Alarm Date: " + alarmDate);
    Serial.println("Alarm Time: " + alarmTime);
    Serial.println("################################");

    // Manually parse the date and time strings (format: "YYYY-MM-DD" and "HH:MM")
    int year = alarmDate.substring(0, 4).toInt();
    int month = alarmDate.substring(5, 7).toInt();
    int day = alarmDate.substring(8, 10).toInt();
    int hour = alarmTime.substring(0, 2).toInt();
    int minute = alarmTime.substring(3, 5).toInt();
    int second = 0;

    // Debug output for parsed values
    Serial.println("Parsed Alarm Date and Time:");
    Serial.println("Year: " + String(year));
    Serial.println("Month: " + String(month));
    Serial.println("Day: " + String(day));
    Serial.println("Hour: " + String(hour));
    Serial.println("Minute: " + String(minute));
    Serial.println("Second: " + String(second));

    // Create a tm struct and set its fields
    struct tm timeStruct = {};
    timeStruct.tm_year = year - 1900;  // tm_year is years since 1900
    timeStruct.tm_mon = month - 1;     // tm_mon is 0-based
    timeStruct.tm_mday = day;
    timeStruct.tm_hour = hour;
    timeStruct.tm_min = minute;
    timeStruct.tm_sec = second;

    // Convert to Unix timestamp
    time_t alarmTimeUnix = mktime(&timeStruct);
    if (alarmTimeUnix == -1) {
      Serial.println("Failed to convert time to Unix timestamp");
      return;
    }

    // Store alarm date, time, and Unix timestamp in preferences
    Config->PutString(ALERT_DATE_, alarmDate);
    Config->PutString(ALERT_TIME_, alarmTime);
    Config->PutULong64(ALERT_TIMESTAMP_SAVED, alarmTimeUnix);

    // Debug output before saving values
    Serial.println("#########################################");
    Serial.println("Saving Alert Date: " + alarmDate);
    Serial.println("Saving Alert Time: " + alarmTime);
    Serial.println("Saving Alert Unix Timestamp: " + String(alarmTimeUnix));
    Serial.println("#########################################");
  }
}
