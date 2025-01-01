#include "ConfigManager.hpp"

ConfigManager::ConfigManager() {
    // Constructor implementation
}

/**
 * @brief Destructor for ConfigManager class.
 * 
 * This is the destructor for the ConfigManager class. It currently has no specific implementation
 * but could be expanded in the future if needed for cleaning up resources.
 */
ConfigManager::~ConfigManager() {
    // Destructor implementation
}

/**
 * @brief Initializes the EEPROM, LED, and Switch pins, and checks the EEPROM initialization status.
 * 
 * This function begins the EEPROM operation by calling `EEPROM.begin()` and checks the size of the EEPROM
 * to verify successful initialization. If the EEPROM size is zero, it reports an initialization failure.
 * If the size is valid, it reports a successful initialization. Additionally, it sets up the LED pin as an
 * output and ensures it's turned off initially, and configures the switch pin as an input.
 * 
 * @note The switch pin is configured for input, and the LED pin is set to output.
 * @note The EEPROM initialization will be checked by verifying the size.
 */
void ConfigManager::begin() {
    // Initialize EEPROM
    EEPROM.begin(EEPROM_SIZE);  
    
    // Get the EEPROM size
    size_t eepromSize = EEPROM.length();
    
    // Check if the EEPROM size is valid
    if (eepromSize == 0) {
        if (DEBUGMODE) {
            Serial.println("Failed to initialize EEPROM (size is 0)");
        }
    } else {
        if (DEBUGMODE) {
            Serial.print("EEPROM initialized successfully with size: ");
            Serial.println(eepromSize);
        }
    }

    // Set the LED pin as output
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);  // Ensure the LED is off initially

    // Set the switch pin as input
    pinMode(SWT_PIN01, INPUT);
    pinMode(SWT_PIN02, INPUT);
}

/** 
 * @brief Sets a flag in the EEPROM to indicate first-time use.
 * 
 * This function writes `FIRSTIME_VAL` to the EEPROM at the specified address to indicate that it’s the first time the device is powered on.
 */
void ConfigManager::setFirstTimeFlag() {
    EEPROM.put(FIRST_TIME_ADD, FIRSTIME_VAL); // Store FIRSTIME_VAL at the specified address
    EEPROM.commit();
    if (DEBUGMODE) {
        Serial.print("First-time flag set to: ");
        Serial.println(FIRSTIME_VAL);
    }
}

/**
 * @brief Checks if it is the first time the system is powered on.
 * 
 * This function reads the flag from the EEPROM. If the value at `FIRST_TIME_ADD` is `FIRSTIME_VAL`, it assumes that the system is powered on for the first time.
 * 
 * @return `true` if the system is powered on for the first time, `false` otherwise.
 */
bool ConfigManager::isFirstTime() {
    int value = EEPROM.read(FIRST_TIME_ADD); // Read the value at the specified address
    if (value == FIRSTIME_VAL) {
        // Flag matches FIRSTIME_VAL, assume first-time use
        return true;
    }
    return false;
}
/** 
 * @brief Sets a flag in the EEPROM to indicate second-time use.
 * 
 * This function writes `SECONDTIME_VAL` to the EEPROM at the specified address to indicate that it’s the second time the device is powered on.
 */
void ConfigManager::setSecondTimeFlag() {
    EEPROM.write(FIRST_TIME_ADD, SECONDTIME_VAL); // Store SECONDTIME_VAL at the specified address
    EEPROM.commit();
    if (DEBUGMODE) {
        Serial.print("Second-time flag set to: ");
        Serial.println(SECONDTIME_VAL);
    }
}

/**
 * @brief Checks if it is the second time the system is powered on.
 * 
 * This function reads the flag from the EEPROM. If the value at `FIRST_TIME_ADD` is `SECONDTIME_VAL`, it assumes that the system is powered on for the second time.
 * 
 * @return `true` if the system is powered on for the second time, `false` otherwise.
 */
bool ConfigManager::isSecondTime() {
    int value = EEPROM.read(FIRST_TIME_ADD); // Read the value at the specified address
    if (value == SECONDTIME_VAL) {
        // Flag matches SECONDTIME_VAL, assume second-time use
        return true;
    }
    return false;
}
/**
 * @brief Sets the trigger time in the EEPROM as a Unix timestamp.
 * 
 * This function stores the Unix timestamp (the number of seconds since January 1, 1970) in the EEPROM.
 * The time is written starting at address 1 using `EEPROM.put()`, and the change is committed using `EEPROM.commit()`.
 * 
 * @param unixTime The Unix timestamp to store in the EEPROM.
 */
void ConfigManager::setTriggerTime(uint32_t unixTime) {
    EEPROM.put(TRIGGER_TIME_START_ADD, unixTime); // Store the time starting at address 1
    EEPROM.commit();
    if (DEBUGMODE) {
        Serial.print("Trigger time set to: ");
        Serial.println(unixTime);
    }
}

/**
 * @brief Retrieves the trigger time stored in the EEPROM.
 * 
 * This function retrieves the Unix timestamp from the EEPROM, which represents the time at which an event should occur.
 * The time is read starting from address 1 using `EEPROM.get()`.
 * 
 * @return The stored Unix timestamp.
 */
uint32_t ConfigManager::getTriggerTime() {
    uint32_t unixTime = 0;
    EEPROM.get(TRIGGER_TIME_START_ADD, unixTime); // Retrieve the time starting at address 1
    return unixTime;
}

/**
 * @brief Clears all data stored in the EEPROM.
 * 
 * This function clears all data in the EEPROM by writing 0xFF to every address in the EEPROM.
 * It uses `EEPROM.write()` in a loop for each memory location and commits the changes with `EEPROM.commit()`.
 */
void ConfigManager::clearEEPROM() {
    for (int i = 0; i < EEPROM_SIZE; i++) {
        EEPROM.write(i, 0xFF);
    }
    EEPROM.commit();
    if (DEBUGMODE) {
        Serial.println("EEPROM cleared");
    }
}

/**
 * @brief Blinks an LED to indicate an action, such as an expired timestamp.
 * 
 * This function configures the LED pin as an output and blinks the LED with a 500ms ON and 500ms OFF cycle.
 * This is typically used to signal some event, such as when the trigger time has been exceeded.
 */
void ConfigManager::blink() {
    pinMode(LED_PIN, OUTPUT);  // Set LED pin as output
    digitalWrite(LED_PIN, HIGH);  // Turn on the LED
    delay(500);                  // Wait for 500ms
    digitalWrite(LED_PIN, LOW);   // Turn off the LED
    delay(500);                  // Wait for 500ms
}

/**
 * @brief Checks if the given time exceeds the stored trigger time and performs an action if it does.
 * 
 * This function compares the given Unix timestamp (`time1`) with the stored trigger time. If `time1` is greater than
 * the stored trigger time, it calls the `blink()` function to blink the LED as an indication that the trigger time has passed.
 * 
 * @param time1 The Unix timestamp to compare with the stored trigger time.
 * @return `true` if `time1` is greater than the stored trigger time, otherwise `false`.
 */
bool ConfigManager::ExpiredCheck(uint32_t time1) {
    if (time1 > getTriggerTime()) {
        // If time1 is greater than the trigger time, do something
        if (DEBUGMODE) {
            Serial.println("Time1 is greater than Time2, action performed.");
        }
        blink();  // Call the blink function to blink the LED
        return true;
    } else {
        // If time1 is not greater than the trigger time, return false
        return false;
    }
}

/**
 * @brief Updates the stored trigger time by adding a specified amount of hours, minutes, and seconds.
 * 
 * This function updates the stored Unix timestamp in the EEPROM by adding the specified hours, minutes, and seconds
 * to the current stored timestamp. The input hours, minutes, and seconds are first converted to seconds, and the 
 * resulting value is added to the existing timestamp.
 * 
 * @param Hour The number of hours to add to the stored time.
 * @param min The number of minutes to add to the stored time.
 * @param sec The number of seconds to add to the stored time.
 */
void ConfigManager::updateStoredTime(uint8_t Hour, uint8_t min, uint8_t sec) {
    uint32_t currentTime = getTriggerTime(); // Get the current stored time (Unix timestamp)

    // Convert hours, minutes, and seconds into total seconds and add to the current time
    uint32_t totalSeconds = (Hour * 3600) + (min * 60) + sec;
    currentTime += totalSeconds; // Add the total seconds to the current Unix timestamp

    setTriggerTime(currentTime); // Update the stored time in EEPROM

    if (DEBUGMODE) {
        Serial.print("Updated time by ");
        Serial.print(Hour);
        Serial.print(" hour(s), ");
        Serial.print(min);
        Serial.print(" minute(s), ");
        Serial.print(sec);
        Serial.println(" second(s).");
    }
}

/**
 * @brief Converts a Unix timestamp to a human-readable time string.
 * 
 * This function converts a Unix timestamp to a human-readable format like "YYYY-MM-DD HH:MM:SS".
 * It uses the `gmtime` function to convert the timestamp into a `tm` structure and then formats it into a string.
 * 
 * @param unixTime The Unix timestamp to convert.
 * @return A human-readable string representing the date and time.
 */
String ConfigManager::unixToHuman(uint32_t unixTime) {
    // Convert Unix timestamp to a tm structure
    struct tm timeInfo;
    gmtime_r((time_t*)&unixTime, &timeInfo);  // Convert to UTC time

    // Format the time as a human-readable string
    char buffer[20];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &timeInfo);
    
    return String(buffer);
}

/**
 * @brief Converts a human-readable time string to a Unix timestamp.
 * 
 * This function takes a time string in the format "YYYY-MM-DD HH:MM:SS" and converts it back to a Unix timestamp.
 * It uses `strptime` to parse the string into a `tm` structure, then uses `mktime` to convert it to a Unix timestamp.
 * 
 * @param humanTime A human-readable string representing the date and time in "YYYY-MM-DD HH:MM:SS" format.
 * @return The corresponding Unix timestamp.
 */
uint32_t ConfigManager::humanToUnix(const String &humanTime) {
    // Parse the human-readable time string into a tm structure
    struct tm timeInfo;
    strptime(humanTime.c_str(), "%Y-%m-%d %H:%M:%S", &timeInfo);

    // Convert the tm structure to Unix timestamp
    return (uint32_t)mktime(&timeInfo);
}

/**
 * @brief Stores a Unix timestamp (32-bit unsigned integer) into the EEPROM at the specified address.
 *
 * This function uses EEPROM.put() to store the 32-bit Unix timestamp in the EEPROM,
 * ensuring simplicity and reliability.
 *
 * @param address The EEPROM address to store the Unix timestamp.
 *                Ensure the address is valid for a 32-bit value.
 * @param timestamp The 32-bit unsigned integer (Unix timestamp) to be stored.
 */
void ConfigManager::storeUnixTimestamp(int address, uint32_t timestamp) {
    EEPROM.put(address, timestamp); // Store the 32-bit Unix timestamp
    EEPROM.commit();                // Commit changes to ensure persistence
}

/**
 * @brief Reads a Unix timestamp (32-bit unsigned integer) from the EEPROM at the specified address.
 *
 * This function uses EEPROM.get() to retrieve the 32-bit Unix timestamp stored at the given address.
 *
 * @param address The EEPROM address to read the Unix timestamp from.
 *                Ensure the address points to a valid location containing a 32-bit value.
 * @return uint32_t The 32-bit unsigned integer (Unix timestamp) retrieved from EEPROM.
 */
uint32_t ConfigManager::readUnixTimestamp(int address) {
    uint32_t timestamp;
    EEPROM.get(address, timestamp); // Retrieve the 32-bit Unix timestamp
    return timestamp;
}

/**
 * @brief Stores a string into the EEPROM at a specified address.
 * 
 * This function stores the length of the string at the starting address and then writes each 
 * character of the string sequentially into the EEPROM. After the string is written, 
 * the changes are committed to the EEPROM. A delay of 1 second is added to ensure the write operation completes.
 * 
 * @param startingAddress The address in the EEPROM where the string will be stored.
 * @param data The string data to store in the EEPROM.
 */
void ConfigManager::storeString(int startingAddress, String data) {
  byte len = data.length();  ///< Get the length of the string

  EEPROM.write(startingAddress, len);  ///< Write the length of the string
  
  // Write each character of the string into EEPROM
  for (int i = 0; i < len; i++) {
    EEPROM.write(startingAddress + 1 + i, data[i]);  ///< Write each character of the string
  }

  EEPROM.commit();  ///< Commit the changes to EEPROM
  delay(1000);  ///< Wait for 1 second to ensure the write operation is complete
}

/**
 * @brief Reads a string from the EEPROM starting from a specified address.
 * 
 * This function reads the length of the string stored in the EEPROM, then sequentially 
 * reads each character of the string into a character array. The character array is then 
 * converted to a `String` object and returned.
 * 
 * @param addrOffset The starting address in EEPROM where the string is stored.
 * @return The string read from the EEPROM.
 */
String ConfigManager::readString(int addrOffset) {
  int newStrLen = EEPROM.read(addrOffset);  ///< Read the length of the stored string
  char data[newStrLen + 1];  ///< Create a character array to hold the string data

  // Read each character of the string from EEPROM
  for (int i = 0; i < newStrLen; i++) {
    data[i] = EEPROM.read(addrOffset + 1 + i);  ///< Read each character of the string
  }
  data[newStrLen] = '\0';  ///< Null-terminate the character array

  return String(data);  ///< Convert the character array to a String object and return it
}

/**
 * @brief Checks if the button connected to SWT_PIN01 is pressed.
 * 
 * This function reads the state of the button connected to `SWT_PIN01`.
 * It returns `true` if the button is pressed (active-low), meaning the pin reads `LOW`.
 * It returns `false` if the button is not pressed, meaning the pin reads `HIGH`.
 * 
 * @return `true` if the button is pressed (active-low), `false` otherwise.
 */
bool ConfigManager::isButtonPressed() {
    return !digitalRead(SWT_PIN01); // Return true if button is pressed (active-low)
}
