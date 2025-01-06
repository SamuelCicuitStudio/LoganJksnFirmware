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
        if(device->isButtonPressed() != false){
        configManager->ResetAPFLag();
            startAccessPoint();
    };

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
                esp_task_wdt_reset(); // Reset the watchdog timer to prevent a system reset
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
    // Endpoint to get both alarm and RTC settings
    server.on("/getSettings", HTTP_GET, [this](AsyncWebServerRequest *request) {
        // Create a JSON document
        DynamicJsonDocument doc(1024);
        
        // Fill the JSON with alarm and RTC data
        JsonObject alarm = doc.createNestedObject("alarm");
        alarm["date"] = configManager->GetString(ALERT_DATE_, "2025-01-01");
        alarm["time"] = configManager->GetString(ALERT_TIME_, "00:00");

        JsonObject rtc = doc.createNestedObject("rtc");
        RTC->update(); // update the containing variables
        rtc["date"] = RTC->getDate();
        rtc["time"] = RTC->getTime();

        // Send the response as JSON
        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response);
    });

    server.on("/setAlarm", HTTP_POST, [this](AsyncWebServerRequest *request) {}, 
        NULL, [this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
            if (DEBUGMODE) Serial.println("Handling Alarm set request");
            esp_task_wdt_reset();
            
            static String jsonData;
            jsonData += String((char*)data, len);

            // Check if we've received the complete data
            if (index + len == total) {
                esp_task_wdt_reset();
                
                if (DEBUGMODE) {
                    Serial.println("Received complete data for Alarm settings, processing...");
                    Serial.println("jsonData content: " + jsonData);
                }

                // Parse the JSON data
                StaticJsonDocument<200> doc;
                DeserializationError error = deserializeJson(doc, jsonData);
                
                if (error) {
                    if (DEBUGMODE) Serial.println("Failed to parse JSON");
                    request->send(400, "application/json", "{\"error\":\"Invalid JSON format\"}");
                    jsonData = "";  // Clear static String for next request
                    return;
                }

                String alarmDate = doc["alarmDate"].as<String>();
                String alarmTime = doc["alarmTime"].as<String>();

                if (alarmDate.isEmpty() || alarmTime.isEmpty()) {
                    request->send(400, "application/json", "{\"error\":\"Missing alarmDate or alarmTime\"}");
                    jsonData = "";  // Clear static String for next request
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

                // Debug output
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
                    if (DEBUGMODE) Serial.println("Failed to convert time to Unix timestamp");
                    request->send(400, "application/json", "{\"error\":\"Invalid alarm time\"}");
                    jsonData = "";  // Clear static String for next request
                    return;
                }

                // Store alarm date, time, and Unix timestamp in preferences
                configManager->PutString(ALERT_DATE_, alarmDate);
                configManager->PutString(ALERT_TIME_, alarmTime);
                configManager->PutULong64(ALERT_TIMESTAMP_SAVED, alarmTimeUnix);
                // Debug output before saving values
                Serial.println("#########################################");
                Serial.println("Saving Alert Date: " + alarmDate);
                Serial.println("Saving Alert Time: " + alarmTime);
                Serial.println("Saving Alert Unix Timestamp: " + String(alarmTimeUnix));
                Serial.println("#########################################");
                esp_task_wdt_reset();  // Reset the watchdog timer to prevent a system reset
                configManager->ResetAPFLag();

                // Respond with success message
                String successResponse = "{\"success\":true}";
                request->send(200, "application/json", successResponse);

                jsonData = "";  // Clear static String for next request
            }
        }
    );

    server.on("/setRTC", HTTP_POST, [this](AsyncWebServerRequest *request) {}, 
        NULL, [this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
            if (DEBUGMODE) Serial.println("Handling RTC set request");
            esp_task_wdt_reset();
            
            static String jsonData;
            jsonData += String((char*)data, len);

            // Check if we've received the complete data
            if (index + len == total) {
                esp_task_wdt_reset();
                
                if (DEBUGMODE) {
                    Serial.println("Received complete data for RTC settings, processing...");
                    Serial.println("jsonData content: " + jsonData);
                }

                // Parse the JSON data
                StaticJsonDocument<200> doc;
                DeserializationError error = deserializeJson(doc, jsonData);
                
                if (error) {
                    if (DEBUGMODE) Serial.println("Failed to parse JSON");
                    request->send(400, "application/json", "{\"error\":\"Invalid JSON format\"}");
                    jsonData = "";  // Clear static String for next request
                    return;
                }

                String rtcDate = doc["rtcDate"].as<String>();
                String rtcTime = doc["rtcTime"].as<String>();

                if (rtcDate.isEmpty() || rtcTime.isEmpty()) {
                    request->send(400, "application/json", "{\"error\":\"Missing rtcDate or rtcTime\"}");
                    jsonData = "";  // Clear static String for next request
                    return;
                }

                // Debug output
                Serial.println("################################");
                Serial.println("RTC Time Set by USER");
                Serial.println("RTC Date: " + rtcDate);
                Serial.println("RTC Time: " + rtcTime);
                Serial.println("################################");

                // Combine date and time into a single string
                String dateTimeString = rtcDate + " " + rtcTime;
                struct tm timeStruct;
                
                // Manually parse the date and time strings (format: "YYYY-MM-DD" and "HH:MM")
                int year = rtcDate.substring(0, 4).toInt();
                int month = rtcDate.substring(5, 7).toInt();
                int day = rtcDate.substring(8, 10).toInt();
                int hour = rtcTime.substring(0, 2).toInt();
                int minute = rtcTime.substring(3, 5).toInt();
                int second = 0;

                // Debug output
                Serial.println("Parsed Date and Time:");
                Serial.println("Year: " + String(year));
                Serial.println("Month: " + String(month));
                Serial.println("Day: " + String(day));
                Serial.println("Hour: " + String(hour));
                Serial.println("Minute: " + String(minute));
                Serial.println("Second: " + String(second));

                // Set the RTC time using the parsed values
                RTC->setRTCTime(year, month, day, hour, minute, second);
                configManager->PutULong64(CURRENT_TIME_SAVED, RTC->getUnixTime());
                configManager->PutULong64(LAST_TIME_SAVED, RTC->getUnixTime());
                configManager->ResetAPFLag();

                // Debug output for current and last saved time
                Serial.println("################################################################");
                Serial.println("Current Time (Unix): " + String(RTC->getUnixTime()));
                Serial.println("Last Saved Time (Unix): " + String(RTC->getUnixTime()));
                Serial.println("################################################################");


                // Respond with success message
                String successResponse = "{\"success\":true}";
                request->send(200, "application/json", successResponse);

                jsonData = "";  // Clear static String for next request
            }
        }
    );

    // Serve static files like icons, CSS, JS, etc.
    server.serveStatic("/icons/", SPIFFS, "/icons/").setCacheControl("max-age=86400");
    esp_task_wdt_reset(); // Reset the watchdog timer to prevent a system reset

    // Start the server
    server.begin();
}

/**
 * @brief Handles incoming reset requests and displays a popup confirmation.
 * 
 * This function processes an incoming reset request. It sends a JavaScript snippet 
 * to the client to display a popup indicating that the reset flag has been set 
 * and the device will restart in 3 seconds.
 * 
 * @param request The incoming web request that triggered the reset action.
 */
void WiFiManager::handleReset(AsyncWebServerRequest* request) {
    if (DEBUGMODE) {
        Serial.println("WiFiManager: Handling Reset request");
    };
    esp_task_wdt_reset(); // Reset the watchdog timer to prevent a system reset

    // JavaScript response to display a popup
    String response = R"rawliteral(
        <script>
            alert("Reset Flag Set. The device will restart in 3 seconds...");
            setTimeout(() => {
                alert("Restarting now...");
            }, 3000);
        </script>
    )rawliteral";

    // Send the JavaScript response to the client
    request->send(200, "text/html", response);

    // Set the reset flag and trigger the system restart
    if (DEBUGMODE) {
        Serial.println("WiFiManager: setting rst flag");
    };
    configManager->PutBool(RESET_FLAG, true);
    esp_task_wdt_reset(); // Reset the watchdog timer to prevent a system reset
    delay(1000);  // Wait briefly
    configManager->RestartSysDelay(3000);  // Restart the system after 3 seconds
}

/**
 * @brief Handles requests to the Restart endpoint and displays a popup confirmation.
 * 
 * This function responds to an incoming restart request by sending a JavaScript 
 * snippet to the client to display a popup message indicating that the system will 
 * restart in 5 seconds.
 * 
 * @param request The incoming web request.
 */
void WiFiManager::handleRestart(AsyncWebServerRequest* request) {
    if (DEBUGMODE) {
        Serial.println("WiFiManager: Handling Restart request");
    };
    esp_task_wdt_reset(); // Reset the watchdog timer to prevent a system reset

    // JavaScript response to display a popup
    String response = R"rawliteral(
        <script>
            alert("Restarting the device in 5 seconds...");
            setTimeout(() => {
                alert("Restarting now...");
            }, 5000);
        </script>
    )rawliteral";

    // Send the JavaScript response to the client
    request->send(200, "text/html", response);
    configManager->PutULong64(CURRENT_TIME_SAVED, RTC->getUnixTime());// save time before restarting
    configManager->PutULong64(LAST_TIME_SAVED, RTC->getUnixTime());// save time before restarting
    // Trigger the system restart
    esp_task_wdt_reset(); // Reset the watchdog timer to prevent a system reset
    delay(1000);  // Wait briefly
    configManager->RestartSysDelayDown(4000);  // Restart the system after 4 seconds
}


/**
 * @brief Handles requests to the Settings endpoint.
 *
 * @param request The incoming web request.
 */
void WiFiManager::handleSettings(AsyncWebServerRequest* request) {
    if (DEBUGMODE) {
        Serial.println("WiFiManager: Handling Settings root request");
    };

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
            configManager->PutULong64(CURRENT_TIME_SAVED, RTC->getUnixTime());// save time before restarting
            configManager->PutULong64(LAST_TIME_SAVED, RTC->getUnixTime());// save time before restarting
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