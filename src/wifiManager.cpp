/**
 * @file WiFiManager.h
 * @brief This file contains the implementation of the WiFiManager class for managing 
 *        Wi-Fi connections and access point functionality on ESP32 devices.
 *
 * The WiFiManager class handles the initialization and connection of Wi-Fi, as well as
 * the creation of an access point (AP) for configuration purposes. It provides methods to 
 * set AP credentials, connect to Wi-Fi, and handle web server requests for managing 
 * Wi-Fi settings and GPIO controls.
 */
#include "WiFiManager.h"


/**
 * @brief Constructor for the WiFiManager class.
 *
 * Initializes the WiFiManager object, setting default values for the access point 
 * credentials and other configurations.
 */
WiFiManager::WiFiManager(ConfigManager* configManager, RTCManager* RTC,Device* device):configManager(configManager),server(80),isAPMode(false), apSSID(DEFAULT_AP_SSID),apPassword(DEFAULT_AP_PASSWORD),RTC(RTC),device(device){}
/**
 * @brief Begins the WiFiManager initialization process.
 *
 * This method mounts the SPIFFS filesystem, checks the configuration for 
 * the connection mode (AP or Wi-Fi), and starts the appropriate connection process.
 */
void WiFiManager::begin() {
    if (DEBUGMODE) {
        Serial.begin(115200);
        Serial.println("###########################################################");
        Serial.println("#                 Starting WIFI Manager                   #");
        Serial.println("###########################################################");
        
        if (!SPIFFS.begin(true)) {
            Serial.println("An error has occurred while mounting SPIFFS");
            return;
        }
        Serial.println("SPIFFS mounted successfully");
        Serial.println("WiFiManager: Begin initialization");
    };
    delay(2000);
    if(!device->isButtonPressed()){
        configManager->ResetAPFLag();
    };
    // Determine the mode to start in (AP or WiFi)
    bool startAP = configManager->GetAPFLag();
    
    // Formatted message
    char text[50]; // Ensure this is large enough to hold your formatted string
    sprintf(text, "WiFiManager: Start mode - %s\n", startAP ? "AP" : "WiFi");
    if (DEBUGMODE) {
        Serial.printf("WiFiManager: Start mode - %s\n", startAP ? "AP" : "WiFi");
    }

// Start in access point mode or connect to WiFi based on the flag
if (startAP) {
    connectToWiFi();
} else {
    startAccessPoint();
}

}


/**
 * @brief Connects to the specified Wi-Fi network.
 *
 * Attempts to connect to the Wi-Fi using stored credentials. If the connection fails, 
 * it defaults to starting the access point.
 */
void WiFiManager::connectToWiFi() {
    String ssid = configManager->GetString(WIFISSID, DEFAULT_WIFI_SSID);
    String password = configManager->GetString(WIFIPASS, DEFAULT_WIFI_PASSWORD);
    // Formatted message
    char text[50]; // Ensure this is large enough to hold your formatted string
    sprintf(text, "WiFiManager:Attempting to connect to WiFi - %s...", ssid);
    
    if (DEBUGMODE) {
        Serial.print("WiFiManager: Attempting to connect to WiFi\n - SSID: ");
        Serial.print(ssid);
        Serial.print(", Password: ");
        Serial.println(password);
    }

    if (ssid == "" || password == "") {
        startAccessPoint();
    } else {
        WiFi.begin(ssid.c_str(), password.c_str());
        unsigned long startAttemptTime = millis();

        if (DEBUGMODE) {
            Serial.println("WiFiManager: Connecting to WiFi...");
        }

        while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
            delay(500);
            if (DEBUGMODE) {
                Serial.print(".");
            }
        }

        if (WiFi.status() == WL_CONNECTED) {
           
            IPAddress localIP = WiFi.localIP();
                // Formatted message
            char text[100]; // Ensure this is large enough to hold your formatted string
            sprintf(text, "WiFiManager: Connected to WiFi IP Address: %d.%d.%d.%d", localIP[0], localIP[1], localIP[2], localIP[3]);
            if (DEBUGMODE) {
                Serial.print("\nWiFiManager: Connected to WiFi,\nIP Address: ");
                Serial.println(WiFi.localIP());
                //setServerCallback();
            }
            //server.begin(); // Start web server
        } else {
            
             if (DEBUGMODE) {
                Serial.println("WiFiManager: Failed to connect to WiFi.\nSwitching to AP mode.");
                configManager->SetAPFLag(); // Set flag to start in AP mode next time
                configManager->RestartSysDelay(3000);
            }
        }
    }
}
/**
 * @brief Starts the access point mode.
 *
 * Sets the ESP32 to access point mode, allowing devices to connect and configure the 
 * Wi-Fi settings.
 */
void WiFiManager::startAccessPoint() {
    if (DEBUGMODE) {
        Serial.println("WiFiManager: Starting Access Point");
    }

    WiFi.disconnect();
    delay(100);

    WiFi.softAP(apSSID.c_str(), apPassword.c_str());
    IPAddress localIP = WiFi.softAPIP();
            // Formatted message
            sprintf(Message, "Connect-IP Address:%d.%d.%d.%d", localIP[0], localIP[1], localIP[2], localIP[3]);
    if (DEBUGMODE) {
        Serial.print("WiFiManager: AP Started - IP Address: ");
        Serial.println(WiFi.softAPIP());
    }

    isAPMode = true;

    setServerCallback();//set the server Callbacks
}

/**
 * @brief Sets up the server callbacks for handling web requests.
 *
 * Configures the web server endpoints for handling root requests, saving Wi-Fi credentials,
 * serving static files, and controlling GPIO.
 */
void WiFiManager::setServerCallback() { 
    // Define the various routes and their corresponding handlers
    server.on("/", HTTP_GET, [this](AsyncWebServerRequest* request) { handleRoot(request); });
    server.on("/wifiCredentialsPage", HTTP_GET, [this](AsyncWebServerRequest* request) { handleSetWiFi(request); });
    server.on("/settings", HTTP_GET, [this](AsyncWebServerRequest* request) { handleSettings(request); });
    server.on("/saveWiFi", HTTP_POST, [this](AsyncWebServerRequest* request) { handleSaveWiFi(request); });
    server.on("/Restart", HTTP_POST, [this](AsyncWebServerRequest* request) { handleRestart(request); });
    server.on("/Reset", HTTP_POST, [this](AsyncWebServerRequest* request) { handleReset(request); });


    // New routes for Alarm and RTC settings
    server.on("/getAlarm", HTTP_GET, [this](AsyncWebServerRequest* request) { handleGetAlarm(request); });
    server.on("/getRTC", HTTP_GET, [this](AsyncWebServerRequest* request) { handleGetRTC(request); });

    server.on("/setAlarm", HTTP_POST, [this](AsyncWebServerRequest* request) { handleSetAlarm(request); });
    server.on("/setRTC", HTTP_POST, [this](AsyncWebServerRequest* request) { handleSetRTC(request); });

    // Serve static files like icons, CSS, JS, etc.
    server.serveStatic("/icons/", SPIFFS, "/icons/").setCacheControl("max-age=86400");

    // Start the server
    server.begin();
}

/**
 * @brief Handles incoming reset requests and initiates a system restart.
 * 
 * This function processes an incoming reset request. It sends a response to 
 * the client indicating that the reset flag has been set. After that, the system 
 * waits for 1 second, then sends another response to the client notifying that 
 * the device will restart in 3 seconds. Finally, the system initiates a restart 
 * process with a 3-second delay.
 * 
 * @param request The incoming web request that triggered the reset action.
 */
void WiFiManager::handleReset(AsyncWebServerRequest* request) {
    if (DEBUGMODE) {  // If debug mode is enabled, print a debug message
        Serial.println("WiFiManager: Handling Reset request");
    }

    // Send a response to the client indicating the reset flag is set
    request->send(200, "text/plain", "Reset Flag Set...");
    configManager->PutBool(RESET_FLAG, true);  // Set the reset flag in configuration manager

    delay(1000);  // Wait for 1 second before sending another response

    // Send another response to the client indicating the system will restart in 3 seconds
    request->send(200, "text/plain", "Resetting the device in 3 seconds...");

    delay(1000);  // Wait for 1 second before triggering the restart

    // Restart the system after the specified delay (3000ms or 3 seconds)
    configManager->RestartSysDelay(3000);
}

/**
 * @brief Handles requests to the Restart endpoint.
 * 
 * This function responds to an incoming restart request by sending a message to 
 * the client indicating that the system is restarting in 5 seconds. After sending 
 * the response, the system will initiate the restart process after the specified 
 * delay of 5 seconds.
 * 
 * @param request The incoming web request.
 */
void WiFiManager::handleRestart(AsyncWebServerRequest* request) {
    if (DEBUGMODE) {
        Serial.println("WiFiManager: Handling Restart request");
    }

    // Send a response to the client indicating the restart is imminent
    request->send(200, "text/plain", "Restarting in 5 seconds...");

    delay(1000);

    // Wait for 5 seconds before restarting the system
    configManager->RestartSysDelay(4000);
}

/**
 * @brief Handles requests to the GetAlarm endpoint.
 *
 * @param request The incoming web request.
 */
void WiFiManager::handleGetAlarm(AsyncWebServerRequest* request) {
    if (DEBUGMODE) {
        Serial.println("WiFiManager: Handling GetAlarm request");
    }

    // Retrieve the saved Unix timestamp for the alarm (stored in the same variable for both alarm and RTC)
    unsigned long long alarmTimeUnix = configManager->GetULong64(ALERT_TIME_SAVED, 0);

    // Check if the alarm timestamp exists (if not, return a default error)
    if (alarmTimeUnix == 0) {
        Serial.println("Error: No alarm time saved.");
        request->send(404, "text/html", "Alarm time not set.");
        return;
    }

    // Convert the unsigned long long to time_t (assuming alarmTimeUnix fits into time_t range)
    time_t alarmTime = static_cast<time_t>(alarmTimeUnix);

    // Convert the Unix timestamp to a human-readable date and time
    struct tm timeStruct;
    gmtime_r(&alarmTime, &timeStruct); // Convert to UTC

    char dateStr[11]; // Format YYYY-MM-DD
    char timeStr[9];  // Format HH:MM:SS
    strftime(dateStr, sizeof(dateStr), "%Y-%m-%d", &timeStruct);
    strftime(timeStr, sizeof(timeStr), "%H:%M:%S", &timeStruct);

    // Prepare the JSON response
    String jsonResponse = "{";
    jsonResponse += "\"date\":\"" + String(dateStr) + "\",";
    jsonResponse += "\"time\":\"" + String(timeStr) + "\"";
    jsonResponse += "}";

    // Send the response as JSON
    request->send(200, "application/json", jsonResponse);
}

/**
 * @brief Handles requests to the GetRTC endpoint.
 *
 * @param request The incoming web request.
 */
void WiFiManager::handleGetRTC(AsyncWebServerRequest* request) {
    if (DEBUGMODE) {
        Serial.println("WiFiManager: Handling GetRTC request");
    }

    // Retrieve the saved Unix timestamp for the RTC (same variable used for alarm time)
    unsigned long long rtcTimeUnix = RTC->getUnixTime();

    // Check if the RTC timestamp exists (if not, return a default error)
    if (rtcTimeUnix == 0) {
        Serial.println("Error: No RTC time saved.");
        request->send(404, "text/html", "RTC time not set.");
        return;
    }

    // Convert the unsigned long long to time_t (assuming rtcTimeUnix fits into time_t range)
    time_t rtcTime = static_cast<time_t>(rtcTimeUnix);

    // Convert the Unix timestamp to a human-readable date and time
    struct tm timeStruct;
    gmtime_r(&rtcTime, &timeStruct); // Convert to UTC

    char dateStr[11]; // Format YYYY-MM-DD
    char timeStr[9];  // Format HH:MM:SS
    strftime(dateStr, sizeof(dateStr), "%Y-%m-%d", &timeStruct);
    strftime(timeStr, sizeof(timeStr), "%H:%M:%S", &timeStruct);

    // Prepare the JSON response
    String jsonResponse = "{";
    jsonResponse += "\"date\":\"" + String(dateStr) + "\",";
    jsonResponse += "\"time\":\"" + String(timeStr) + "\"";
    jsonResponse += "}";

    // Send the response as JSON
    request->send(200, "application/json", jsonResponse);
}

/**
 * @brief Handles requests to the SetAlarm endpoint.
 *
 * @param request The incoming web request.
 */
void WiFiManager::handleSetAlarm(AsyncWebServerRequest* request) {
    if (DEBUGMODE) {
        Serial.println("WiFiManager: Handling SetAlarm request");
    };

    if (request->hasParam("alarmDate", true) && request->hasParam("alarmTime", true)) {
        String alarmDate = request->getParam("alarmDate", true)->value();
        String alarmTime = request->getParam("alarmTime", true)->value();
        
        // Debug output
        Serial.println("Alarm Date: " + alarmDate);
        Serial.println("Alarm Time: " + alarmTime);

        // Combine date and time into a full datetime string in the format "yyyy-MM-dd HH:mm:ss"
        String dateTimeString = alarmDate + " " + alarmTime;

        // Convert to Unix timestamp
        struct tm timeStruct;
        strptime(dateTimeString.c_str(), "%Y-%m-%d %H:%M:%S", &timeStruct);  // Parse datetime
        time_t alarmTimeUnix = mktime(&timeStruct);  // Convert to Unix timestamp

        // Write the Unix timestamp to the preferences partition
        configManager->PutULong64(ALERT_TIME_SAVED, alarmTimeUnix);
        configManager->ResetAPFLag();
        // Debug output
        Serial.println("Alarm Unix Time: " + String(alarmTimeUnix));
    }

    // Respond with HTML and JavaScript for popup message
    String htmlResponse = "<html><body>";
    htmlResponse += "<script type='text/javascript'>";
    htmlResponse += "alert('Alarm Saved');";  // JavaScript to show alert
    htmlResponse += "</script>";
    htmlResponse += "</body></html>";

    // Send the response with the popup
    request->send(200, "text/html", htmlResponse);
}


/**
 * @brief Handles requests to the Set RTC endpoint.
 *
 * @param request The incoming web request.
 */
void WiFiManager::handleSetRTC(AsyncWebServerRequest* request) {
    if (DEBUGMODE) {
        Serial.println("WiFiManager: Handling Set RTC request");
    }

    if (request->hasParam("alarmDate", true) && request->hasParam("alarmTime", true)) {
        String rtcDate = request->getParam("rtcDate", true)->value();
        String rtcTime = request->getParam("rtcTime", true)->value();
        
        // Debug output
        Serial.println("RTC Date: " + rtcDate);
        Serial.println("RTC Time: " + rtcTime);

        String dateTimeString = rtcDate + " " + rtcTime;
        struct tm timeStruct;
        strptime(dateTimeString.c_str(), "%Y-%m-%d %H:%M:%S", &timeStruct);
        time_t rtcTimeUnix = mktime(&timeStruct);

        // Write the Unix timestamp to the preferences partition
        RTC->setUnixTime(rtcTimeUnix);// set the RTC time
        configManager->PutULong64(CURRENT_TIME_SAVED, rtcTimeUnix);
         configManager->PutULong64(LAST_TIME_SAVED, rtcTimeUnix);
        configManager->ResetAPFLag();
        // Debug output
        Serial.println("RTC Unix Time: " + String(rtcTimeUnix));
    }

    // Respond with HTML and JavaScript for popup message
    String htmlResponse = "<html><body>";
    htmlResponse += "<script type='text/javascript'>";
    htmlResponse += "alert('RTC time Saved');";  // JavaScript to show alert
    htmlResponse += "</script>";
    htmlResponse += "</body></html>";

    // Send the response with the popup
    request->send(200, "text/html", htmlResponse);
}
/**
 * @brief Handles requests to the Settings endpoint.
 *
 * @param request The incoming web request.
 */
void WiFiManager::handleSettings(AsyncWebServerRequest* request) {
    if (DEBUGMODE) {
        Serial.println("WiFiManager: Handling Settings root request");
    }

    request->send(SPIFFS, "/BoardSetting.html", "text/html");
}
/**
 * @brief Handles requests to the root endpoint.
 *
 * @param request The incoming web request.
 */
void WiFiManager::handleRoot(AsyncWebServerRequest* request) {
    if (DEBUGMODE) {
        Serial.println("WiFiManager: Handling welcome root request");
    }

    request->send(SPIFFS, "/welcome.html", "text/html");
}

/**
 * @brief Handles requests for the Wi-Fi credentials page.
 *
 * @param request The incoming web request.
 */
void WiFiManager::handleSetWiFi(AsyncWebServerRequest* request) {
    if (DEBUGMODE) {
        Serial.println("WiFiManager: Handling set wifi request");
    }

    request->send(SPIFFS, "/wifiCredentialsPage.html", "text/html");
}
/**
 * @brief Handles saving the Wi-Fi credentials.
 *
 * @param request The incoming web request containing the SSID and password.
 */
void WiFiManager::handleSaveWiFi(AsyncWebServerRequest* request) {
    if (DEBUGMODE) {
        Serial.println("WiFiManager: Handling save WiFi request");
    }

    if (request->hasParam("ssid", true) && request->hasParam("password", true)) {
        String ssid = request->getParam("ssid", true)->value();
        String password = request->getParam("password", true)->value();

        if (DEBUGMODE) {
            Serial.print("WiFiManager: Received credentials - SSID: ");
            Serial.print(ssid);
            Serial.print(", Password: ");
            Serial.println(password);
        }

        if (ssid != "" && password != "") {
            // Formatted message
            char text[50]; // Ensure this is large enough to hold your formatted string
            sprintf(text, "WiFiManager: Saving Wifi Credentials...");
            configManager->PutString(WIFISSID, ssid);
            configManager->PutString(WIFIPASS, password);
            configManager->ResetAPFLag();
            request->send(SPIFFS, "/thankyou_page.html", "text/html");
            sprintf(text, "WiFiManager: Device Restarting in 3 Sec");
            configManager->RestartSysDelay(3000);
        } else {
            request->send(400, "text/plain", "Invalid SSID or Password.");
        }
    } else {
        request->send(400, "text/plain", "Missing parameters.");
    }
}
/**
 * @brief Gets the Wi-Fi signal strength as a percentage.
 *
 * @return The signal strength percentage.
 */
uint8_t WiFiManager::getSignalStrengthPercent() {
    int rssi = WiFi.RSSI();  // Get RSSI value
    // Convert RSSI to a percentage
    if (rssi <= -100) {
        return 0;  // Very weak signal
    } else if (rssi >= -50) {
        return 100;  // Excellent signal
    } else {
        return 2 * (rssi + 100);  // Convert to percentage
    }
}

/**
 * @brief Checks if the ESP32 is still connected to the Wi-Fi network.
 * 
 * @return true if connected to Wi-Fi, false otherwise.
 */
bool WiFiManager::isStillConnected() {
    return WiFi.status() == WL_CONNECTED;
}