#ifndef WEBPAGE_HPP
#define WEBPAGE_HPP
#include <Arduino.h>
const char* webpage_html = R"(
<html>
  <head>
    <style>
      body {
        font-family: Arial, sans-serif;
        margin: 0;
        padding: 0;
        display: flex;
        justify-content: center;
        align-items: center;
        height: 100vh;
        background-color: #f4f4f4;
      }
      form {
        background-color: white;
        padding: 20px;
        border-radius: 8px;
        box-shadow: 0 4px 8px rgba(0, 0, 0, 0.1);
        width: 300px;
      }
      input[type="text"], input[type="password"], input[type="date"] {
        width: 100%;
        padding: 10px;
        margin: 10px 0;
        border-radius: 4px;
        border: 1px solid #ccc;
      }
      button {
        width: 100%;
        padding: 10px;
        background-color: #4CAF50;
        color: white;
        border: none;
        border-radius: 4px;
        cursor: pointer;
      }
      button:hover {
        background-color: #45a049;
      }
    </style>
    <script>
      function setUnixTimestamp() {
        const dateInput = document.getElementById("dateInput").value;
        if (dateInput) {
          const date = new Date(dateInput);
          const timestamp = Math.floor(date.getTime() / 1000); // Convert to Unix timestamp (seconds)
          document.getElementById("timestamp").value = timestamp;
        }
      }
    </script>
  </head>
  <body>
    <div>
      <h2>ESP Settings</h2>
      <form action="/save" method="POST">
        Wi-Fi SSID: <input type="text" name="ssid" value=""><br><br>
        Wi-Fi Password: <input type="password" name="password" value=""><br><br>
        
        <!-- Date input with calendar popup -->
        Select Date: <input type="date" id="dateInput" onchange="setUnixTimestamp() /><br><br>
        
        <!-- Hidden text field for timestamp -->
        Timestamp: <input type="text" id="timestamp" name="timestamp" value="" readonly /><br><br>
        
        <button type="submit">Save</button>
      </form>
    </div>
  </body>
</html>
)";

#endif
