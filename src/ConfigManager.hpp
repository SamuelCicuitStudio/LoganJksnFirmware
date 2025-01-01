#ifndef CONFIG_MANAGER_HPP
#define CONFIG_MANAGER_HPP

#include <EEPROM.h>
#include <Arduino.h>
#include "Config.h"
#include <ESP8266WiFi.h>
#include <time.h>

class ConfigManager {
public:
    // Constructor and Destructor
    ConfigManager();
    ~ConfigManager();

    // EEPROM Initialization and Management
    void begin();                        // Initialize EEPROM
    void clearEEPROM();                  // Clear all EEPROM data

    // First-time flag handling
    void setFirstTimeFlag();             // Set the first-time flag
    bool isFirstTime();                  // Check the first-time flag

    // Second-time flag handling
    void setSecondTimeFlag();            // Set the second-time flag
    bool isSecondTime();                 // Check the second-time flag

    // Trigger time handling
    void setTriggerTime(uint32_t unixTime); // Store the trigger time in Unix format
    uint32_t getTriggerTime();           // Retrieve the trigger time

    // Time-related functions
    bool ExpiredCheck(uint32_t time1);   // Compare Unix timestamps and check expiration
    void blink();                        // Blink the LED (when expired check occurs)
    void updateStoredTime(uint8_t Hour, uint8_t min, uint8_t sec); // Update stored time by adding hours, minutes, and seconds

    // Conversion functions for Unix and human-readable time
    String unixToHuman(uint32_t unixTime); // Convert Unix timestamp to human-readable time
    uint32_t humanToUnix(const String &humanTime); // Convert human-readable time to Unix timestamp

    // EEPROM functions for Unix timestamps
    void storeUnixTimestamp(int address, uint32_t timestamp); // Store Unix timestamp into EEPROM
    uint32_t readUnixTimestamp(int address); // Read Unix timestamp from EEPROM

    // EEPROM functions for strings
    void storeString(int startingAddress, String data); // Store string into EEPROM
    String readString(int addrOffset); // Read string from EEPROM

    // Button press detection
    bool isButtonPressed(); // Check if a button is pressed

private:
    // Additional private members if necessary
};

#endif // CONFIG_MANAGER_HPP
