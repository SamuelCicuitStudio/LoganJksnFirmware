#ifndef CONFIG_H
#define CONFIG_H

// ==================================================
// Device Identification Keys
// ==================================================
#define DEVICE_NAME "DEVNAM"                           ///< Device name
#define DEVICE_ID "DEVID"                              ///< Unique device identifier

#define WIFISSID "WFSSID"                             ///< Wi-Fi network SSID
#define WIFIPASS "WFPASS"                             ///< Wi-Fi network password
#define RESET_FLAG "RSTFLG"                           ///< Key to trigger a reset operation

// RTC Time and Date Keys (Unix Timestamp Format)
#define CURRENT_TIME_SAVED "CURTIM"                   ///< Key for saving the current time
#define LAST_TIME_SAVED "LSTTIM"                      ///< Key for saving the last recorded time
#define ALERT_TIMESTAMP_SAVED "ALRTIM"                ///< Key for saving the alert timestamp
#define ALERT_DATE_ "DATE"                             ///< Key for saving the alert date
#define ALERT_TIME_ "TIME"                             ///< Key for saving the alert time
#define LED_STATE "LEDSTA"                            ///< Key for saving the LED state

#define WIFI_TIMEOUT 120000                           ///< Wi-Fi connection timeout (2 minutes in milliseconds)
#define TIME_ERROR_THRESHOLD 5400                     ///< Time error threshold (in seconds)
#define DEEPSLEEP_TIME 60000                          ///< Deep sleep timeout (in milliseconds)

// ==================================================
// Default Values
// ==================================================

// Default Device Information
#define DEFAULT_DEVICE_NAME "DefaultDevice"           ///< Default device name
#define DEFAULT_DEVICE_ID "00000001"                  ///< Default unique device identifier

// Default Wi-Fi Access Point Credentials
#define DEFAULT_AP_SSID "Alarm DEVICE"          ///< Default Access Point SSID
#define DEFAULT_AP_PASSWORD "12345678"                ///< Default Access Point password

// Default Wi-Fi Credentials
#define DEFAULT_WIFI_SSID "DefaultWifi"               ///< Default Wi-Fi SSID
#define DEFAULT_WIFI_PASSWORD "12345678"             ///< Default Wi-Fi password

// Default RTC Values
#define DEFAULT_CURRENT_TIME_SAVED 1736121600        ///< Default current time (Unix timestamp example)
#define DEFAULT_LAST_TIME_SAVED 1736121600           ///< Default last time saved (Unix timestamp example)
#define DEFAULT_LED_STATE false                      ///< Default LED state (false for OFF, true for ON)
#define DEFAULT_ALERT_TIME_SAVED 0                  ///< Default alert time (Unix epoch timestamp)

// ==================================================
// General Configuration
// ==================================================

#define DEBUGMODE 1                                   ///< Debug mode flag (1 = enabled, 0 = disabled)
#define SERIAL_BAUD_RATE 115200                       ///< Baud rate for serial communication
#define CONFIG_PARTITION "config"                     ///< Configuration storage partition name

// ==================================================
// Pin Configuration
// ==================================================
#define LED_GREEN_PIN 2                               ///< Pin for green LED (status indicator)
#define BUZZ_PIN 14                                   ///< Pin for buzzer (mode indicator)
#define SWITCH_PIN 0                                  ///< Pin for switch AP mode
#define PROG_SWITCH_PIN 4                             ///< Pin for switch Serial ProgMode
// ==================================================
// Time Configuration
// ==================================================

// Time Offset (in seconds)
#define TIMEOFFSET 3600                               ///< Time offset for UTC+1 (3600 seconds)
#define NTP_SERVER "pool.ntp.org"                     ///< NTP server for time synchronization
#define NTP_UPDATE_INTERVAL 60000                     ///< NTP update interval in milliseconds (default 1 minute)

// ==================================================
// End of Configuration
// ==================================================
#endif // CONFIG_H
