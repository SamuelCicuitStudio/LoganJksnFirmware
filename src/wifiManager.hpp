#ifndef WIFIMANAGER_HPP
#define WIFIMANAGER_HPP

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "ConfigManager.hpp"
#include <WiFiUdp.h>
#include <NTPClient.h>
#include "RTCWakeupManager.hpp"

// External reference to the webpage HTML content
extern const char* webpage_html;

class WifiManager {
public:
    WifiManager(ConfigManager * device,ESP8266WebServer* server,RTCWakeupManager * Wake);                         // Constructor
    void begin();                          // Start Wi-Fi and the web server
    void handleClient();                   // Handle incoming web requests
    String getHTMLPage();                  // Return the HTML page
    bool connectToWiFi();                  // Connect to Wi-Fi and return true if successful
    unsigned long getUnixTimestampFromWiFi();  // Get the Unix timestamp from Wi-Fi (current time)

private:
    RTCWakeupManager * Wake;
    ConfigManager * device;
    ESP8266WebServer* server;               // Web server instance
};

#endif
