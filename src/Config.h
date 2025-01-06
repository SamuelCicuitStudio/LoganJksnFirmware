#ifndef CONFIG_H
#define CONFIG_H



// ==================================================
// Device Keys
// ==================================================
#define DEVICE_NAME "DEVNAM"                           ///< Name of the device
#define DEVICE_ID "DEVID"                              ///< Unique identifier for the device

#define APWIFIMODE_FLAG "STRAP"                        ///< Indicates if the device is in AP (Access Point) Wi-Fi mode
#define WIFISSID "WFSSID"                             ///< Wi-Fi network SSID key
#define WIFIPASS "WFPASS"                             ///< Wi-Fi network password key
#define RESET_FLAG "RSTFLG"                               ///< Key to trigger a reset operation

// RTC Time and Date Keys (Unix Format)
#define CURRENT_TIME_SAVED "CURTIM"                    ///< Key for saving the current time
#define LAST_TIME_SAVED "LSTTIM"                       ///< Key for saving the last recorded time
#define ALERT_TIMESTAMP_SAVED "ALRTIM"                       ///< Key for saving the alert time unix
#define ALERT_DATE_ "DATE"                       ///< Key for saving the alert time unix
#define ALERT_TIME_ "TIME"                       ///< Key for saving the alert time unix
#define LED_STATE "LEDSTA"                             ///< Key for saving the LED state

#define FIRST_TIME "FRSTIM"                             ///< Key for saving first time flag

#define WIFI_TIMEOUT 120000  // Timeout period set to 2 minutes (120,000 milliseconds)
#define TIME_ERROR_THRESHOLD 5400
#define DEEPSLEEP_TIME 3600000000
// ==================================================
// Default Values
// ==================================================

// Default Device Information
#define DEFAULT_DEVICE_NAME "DefaultDevice"            ///< Default device name
#define DEFAULT_DEVICE_ID "00000001"                   ///< Default unique identifier for the device

// Default Wi-Fi Access Point (AP) Credentials
#define DEFAULT_AP_SSID "ALARM DEVICE"                 ///< Default Access Point SSID
#define DEFAULT_AP_PASSWORD "12345678"                 ///< Default Access Point password

// Default Wi-Fi Credentials
#define DEFAULT_WIFI_SSID "Techlancer"                 ///< Default Wi-Fi SSID
#define DEFAULT_WIFI_PASSWORD "12345678"               ///< Default Wi-Fi password

// Default RTC Values
#define DEFAULT_CURRENT_TIME_SAVED 0        ///< Default current time (example in Unix timestamp format)
#define DEFAULT_LAST_TIME_SAVED 0           ///< Default last time saved (example in Unix timestamp format)
#define DEFAULT_LED_STATE false                        ///< Default LED state (e.g., "ON" or "OFF")
#define DEFAULT_ALERT_TIME_SAVED 0  // Default alert time (e.g., Unix epoch)

//Default Flag Device
#define  DEFAULT_FIRST_TIME_FLAG 455

// ==================================================
// General Configuration
// ==================================================

#define DEBUGMODE 1                                    ///< Debug mode flag (1 = enabled, 0 = disabled)
#define SERIAL_BAUD_RATE 115200                        ///< Baud rate for serial communication
#define CONFIG_PARTITION "config"                      ///< Configuration storage partition name

// ==================================================
// Pin Configuration
// ==================================================
#define LED_GREEN_PIN 4                                ///< Pin for the green LED (status indicator)
#define BUZZ_PIN 14                                    ///< Pin for the buzzer (mode indicator)
#define SWITCH_PIN 0                                  ///< Pin for the switch

// ==================================================
// Time config
// ==================================================

// Time Offset (in seconds)
#define TIMEOFFSET    3600  // Example: UTC+1 hour (3600 seconds)
// Optional: NTP server for time synchronization (default is "pool.ntp.org")
#define NTP_SERVER    "pool.ntp.org"
// Optional: Update interval in milliseconds for NTP client (default is 60000 ms, or 1 minute)
#define NTP_UPDATE_INTERVAL 60000
// ==================================================
// End of Configuration
// ==================================================
#endif // CONFIG_H
