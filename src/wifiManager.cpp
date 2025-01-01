#include "wifiManager.hpp"
#include "webpage.hpp"  // Include the webpage.h for raw HTML

// Define NTP server and update interval
const char* ntpServer = "pool.ntp.org";
const long utcOffsetInSeconds = TIME_UTC_OFFSET;  // Adjust for your timezone (e.g., UTC+1 for Central European Time)

WiFiUDP udp;  // UDP object for communication
NTPClient timeClient(udp, ntpServer, utcOffsetInSeconds);


WifiManager::WifiManager(ConfigManager * device,ESP8266WebServer* server,RTCWakeupManager * Wake) : server(server), device(device), Wake(Wake) {
    // Constructor does nothing for now
    Serial.print("Wifi instance created ");
}

/**
 * @brief Initializes the Wi-Fi and web server.
 * Connects to Wi-Fi and starts the web server to serve the page.
 */
void WifiManager::begin() {
    // Set the ESP8266 in Access Point mode (Hotspot)
    WiFi.mode(WIFI_AP);
    WiFi.softAP(DEFAULT_AP_SSID, DEFAULT_AP_PASSWORD);  // Change the name and password as needed

    // Print the IP address of the ESP Hotspot
    Serial.print("Hotspot IP Address: ");
    Serial.println(WiFi.softAPIP());  // Show the ESP's AP IP address

    // Configure web server routes
    server->on("/", HTTP_GET, [this]() { 
        server->send(200, "text/html", webpage_html); 
    });

    server->on("/save", HTTP_POST, [this]() { 
        String ssid = server->arg("ssid");
        String password = server->arg("password");
        unsigned long timestamp = server->arg("timestamp").toInt();
        
        // Save Wi-Fi credentials and timestamp to EEPROM
        device->storeString(WIFI_SSID_ADDR, ssid);
        device->storeString(WIFI_PASS_ADDR, password);
        device->storeUnixTimestamp(TRIGGER_TIME_START_ADD, timestamp);
        Serial.print("Setting First time flag");
        device->setFirstTimeFlag(); // Mark as first-time flag
        Serial.print("device powering down in 2 second...");
        delay(2000);
        Wake->enterDeepSleep(2000000);// simulate power down for 2 second
        // Send response back to user
        server->send(200, "text/html", "<html><body style='text-align:center;'><h2>Settings Saved!</h2><a href='/'>Back</a></body></html>");

    });

    // Start the web server
    server->begin();
    Serial.println("Web server started");
}


/**
 * @brief Handles client requests.
 */
void WifiManager::handleClient() {
    server->handleClient();
}


/**
 * @brief Returns the HTML page with saved credentials and timestamp.
 * @return HTML page as a string.
 */
String WifiManager::getHTMLPage() {
    String html = "<html><body style='text-align:center;'>"
                  "<h2>ESP Settings</h2>"
                  "<form action='/save' method='POST'>";
    
    // Retrieve saved Wi-Fi credentials and timestamp
    String savedSSID = device->readString(WIFI_SSID_ADDR);
    String savedPassword = device->readString(WIFI_PASS_ADDR);
    unsigned long savedTimestamp = device->readUnixTimestamp(TRIGGER_TIME_START_ADD);

    html += "Wi-Fi SSID: <input type='text' name='ssid' value='" + savedSSID + "'><br><br>";
    html += "Wi-Fi Password: <input type='password' name='password' value='" + savedPassword + "'><br><br>";
    html += "Timestamp: <input type='text' name='timestamp' value='" + String(savedTimestamp) + "'><br><br>";
    html += "<button type='submit'>Save</button>";
    html += "</form></body></html>";

    return html;
}

/**
 * @brief Connects the device to Wi-Fi using saved credentials.
 * @return true if successfully connected to Wi-Fi, false otherwise.
 */
bool WifiManager::connectToWiFi() {
    String ssid = device->readString(WIFI_SSID_ADDR);
    String password = device->readString(WIFI_PASS_ADDR);

    if (ssid == "" || password == "") {
        Serial.println("Wi-Fi credentials are empty!");
        return false;
    }

    WiFi.begin(ssid.c_str(), password.c_str());  // Attempt to connect to Wi-Fi

    Serial.print("Connecting to Wi-Fi");
    unsigned long startAttemptTime = millis();

    // Try connecting for 10 seconds
    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
        delay(500);
        Serial.print(".");
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nConnected to Wi-Fi");
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());
        return true;
    } else {
        Serial.println("\nFailed to connect to Wi-Fi");
        return false;
    }
}

/**
 * @brief Connects to Wi-Fi and retrieves the current Unix timestamp from an NTP server.
 *
 * This function attempts to connect to the Wi-Fi network using credentials stored in EEPROM. Once connected, 
 * it uses an NTP client to fetch the current Unix timestamp (the number of seconds since January 1, 1970). 
 * The timestamp is printed to the Serial Monitor for debugging purposes.
 * 
 * @return unsigned long The current Unix timestamp (seconds since Jan 1, 1970). 
 *         Returns 0 if the Wi-Fi connection fails.
 */
unsigned long WifiManager::getUnixTimestampFromWiFi() {
    // Connect to Wi-Fi before requesting time
    if (connectToWiFi()) {
        timeClient.begin(); // Start the NTP client
        timeClient.update(); // Update the time

        unsigned long currentUnixTimestamp = timeClient.getEpochTime();  // Get Unix timestamp
        Serial.print("Unix Timestamp: ");
        Serial.println(currentUnixTimestamp); // Print the Unix timestamp to the Serial Monitor

        return currentUnixTimestamp;
    } else {
        Serial.println("Wi-Fi not connected. Cannot retrieve timestamp.");
        return 0;  // Return 0 if Wi-Fi connection fails
    }
}
