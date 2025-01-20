#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include "ConfigManager.h"
#include "RTCManager.h"
#include "Device.h"



class WiFiManager {
public:
    // Constructor
    WiFiManager(ConfigManager* configManager, RTCManager* RTC,Device* device);
    // Destructor to clean up allocated managers


    void begin();
    void setServerCallback();
    uint8_t getSignalStrengthPercent();
    char Message[100];
    bool isStillConnected();
    void connectToWiFi();

private:
    
    void startAccessPoint();
    void handleRoot(AsyncWebServerRequest* request);
    void handleSettings(AsyncWebServerRequest* request);
    void handleSetWiFi(AsyncWebServerRequest* request);
    void handleSaveWiFi(AsyncWebServerRequest* request);
    void handleRestart(AsyncWebServerRequest* request) ;
    void handleReset(AsyncWebServerRequest* request) ;
    

    ConfigManager* configManager;
    RTCManager* RTC;
    Device* device;
    AsyncWebServer server;
    bool isAPMode;
    String apSSID;
    String apPassword;
    WiFiUDP ntpUDP;
};


#endif // WIFI_MANAGER_H