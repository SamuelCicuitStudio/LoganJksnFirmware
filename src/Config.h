#ifndef CONFIG_H
#define CONFIG_H

// ==================================================
// Device Configuration
// ==================================================

/**
 * @brief Device configuration details including name, ID, and firmware version.
 */
#define DEVICE_NAME             "EcoGrow"           ///< @brief The name of the device.
#define DEVICE_ID               "EcoGrow001"        ///< @brief Unique identifier for the device.
#define FIRMWARE_VERSION        "FIRVER"            ///< @brief Firmware version identifier.
#define DEFAULT_FIRMWARE_VERSION "1.0.0"            ///< @brief Default firmware version if not updated.

// ==================================================
// Wi-Fi Configuration
// ==================================================
#define TIME_UTC_OFFSET 3600

/**
 * @brief Default Wi-Fi Access Point (AP) credentials.
 */
#define DEFAULT_AP_SSID         "EcoGrow"           ///< @brief Default SSID for the Access Point mode.
#define DEFAULT_AP_PASSWORD     "0123456789"        ///< @brief Default password for the Access Point mode.

/**
 * @brief Default Wi-Fi credentials for connecting to a network.
 */
#define DEFAULT_WIFI_SSID       "EcoGrow"           ///< @brief Default SSID for client Wi-Fi connection.
#define DEFAULT_WIFI_PASSWORD   "0123456789"        ///< @brief Default password for client Wi-Fi connection.

// ==================================================
// Debug and Serial Configuration
// ==================================================

/**
 * @brief Debug mode configuration.
 * Set to 1 to enable debug output, 0 to disable.
 */
#define DEBUGMODE               1                   ///< @brief Enable or disable debug mode (1: enabled, 0: disabled).

/**
 * @brief Serial communication baud rate.
 */
#define SERIAL_BAUD_RATE        115200              ///< @brief Baud rate for serial communication, typically 115200.

// ==================================================
// Firmware Update Configuration
// ==================================================

/**
 * @brief Interval for checking firmware updates.
 * Time is in milliseconds (1 hour = 3600000 ms).
 */
#define DEVICE_UPDATE_CHECK_INTERVAL 3600000000      ///< @brief Interval for firmware update check (every 1 hour).

// ==================================================
// Hardware Configuration (GPIO Pins)
// ==================================================

/**
 * @brief GPIO pin assignments for switches and LEDs.
 */
#define SWT_PIN01                 19                  ///< @brief GPIO pin number for Switch 1.
#define SWT_PIN02                 19                  ///< @brief GPIO pin number for Switch 2.
#define LED_PIN                 19                  ///< @brief GPIO pin number for LED (same as Switch 1 for simplicity).

// ==================================================
// EEPROM Configuration
// ==================================================

/**
 * @brief EEPROM related configuration including size and address assignments.
 */
#define EEPROM_SIZE             256                 ///< @brief Total EEPROM size in bytes.
#define TRIGGER_TIME_START_ADD  1                   ///< @brief Start address for storing trigger time in EEPROM.
#define LAST_STORED_TIME_START_ADD 50              ///< @brief Start address for storing the last stored time in EEPROM.
#define WIFI_SSID_ADDR          72                  ///< @brief EEPROM address for storing Wi-Fi SSID.
#define WIFI_PASS_ADDR          125                 ///< @brief EEPROM address for storing Wi-Fi password.

#define FIRST_TIME_ADD   171
#define FIRSTIME_VAL  109
#define SECONDTIME_VAL 83
#endif // CONFIG_H
