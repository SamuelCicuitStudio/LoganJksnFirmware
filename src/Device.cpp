#include "Device.h"

Device::Device() {
    _lastBlinkTime = 0;
    _ledState = false;
}

/**
 * @brief Initializes the GPIO pins for the LED, button, and buzzer.
 * Sets the pin modes and initial state for the components.
 */
void Device::begin() {
    
    pinMode(LED_GREEN_PIN, OUTPUT);// Initialize the LED pin as output
    pinMode(SWITCH_PIN, INPUT_PULLUP);  // Assuming the switch is connected to ground
    pinMode(BUZZ_PIN, OUTPUT);// Initialize the buzzer pin as output
    digitalWrite(BUZZ_PIN, LOW);// Set initial state for buzzer (off)
}

/**
 * @brief Blinks the LED at the specified interval.
 * Toggles the LED state every time the specified interval has passed.
 * This version uses delay() and blocks execution during the LED blink.
 *
 * @param interval The time interval (in milliseconds) for the LED blink.
 */
void Device::blinkLED(unsigned long interval) {
    // Toggle LED state
    _ledState = !_ledState;
    digitalWrite(LED_GREEN_PIN, _ledState ? HIGH : LOW);
    
    // Delay for the specified interval before toggling again
    delay(interval);
}


/**
 * @brief Checks if the button (switch) is pressed.
 * Reads the state of the switch pin and returns true if the button is pressed (LOW state).
 *
 * @return true if the button is pressed, false otherwise.
 */
bool Device::isButtonPressed() {
    // Check if button (switch) is pressed (LOW because of INPUT_PULLUP)
    return digitalRead(SWITCH_PIN) == LOW;
}

/**
 * @brief Controls the buzzer state.
 * Turns the buzzer on or off based on the provided state.
 *
 * @param state true to turn the buzzer on, false to turn it off.
 */
void Device::controlBuzzer(bool state) {
    // Turn buzzer on or off
    digitalWrite(BUZZ_PIN, state ? HIGH : LOW);
}


/**
 * @brief Puts the device into deep sleep mode for a given amount of time.
 *
 * @param sleepDuration The duration (in milliseconds) for the device to remain in deep sleep.
 */
void Device::deepSleep(unsigned long sleepDuration) {
    // Print debug information to Serial

    Serial.print("The system will sleep for ");
    Serial.print(sleepDuration / 1000); // Convert milliseconds to seconds
    Serial.println(" seconds.");

    // Convert the sleep duration from milliseconds to microseconds
    unsigned long sleepTimeInMicroseconds = sleepDuration * 1000;

    // Set the deep sleep duration
    esp_sleep_enable_timer_wakeup(sleepTimeInMicroseconds);

    // Optional: Configure additional wakeup sources
    // esp_sleep_enable_ext0_wakeup(GPIO_NUM_XX, HIGH); // Example GPIO wakeup

    // Flush Serial buffer and notify before entering sleep
    Serial.println("Entering deep sleep now...");
    Serial.flush();
    delay(100); // Give time for Serial output to complete

    // Enter deep sleep
    esp_deep_sleep_start();
}


/**
 * @brief Determines the cause of the wake-up and returns an integer based on the source.
 * This function handles all possible wake-up causes from deep sleep.
 *
 * @return An integer representing the cause of the wake-up.
 *         - 0: Timer wakeup
 *         - 1: GPIO wakeup (external signal)
 *         - 2: Touchpad wakeup
 *         - 3: ULP program wakeup
 *         - 4: GPIO (light sleep only)
 *         - 5: UART wakeup (light sleep only)
 *         - 6: WiFi wakeup (light sleep only)
 *         - 7: COCPU wakeup
 *         - 8: COCPU trap trigger wakeup
 *         - 9: BT wakeup (light sleep only)
 *         - -1: Undefined or unknown wake-up cause
 */
int Device::getWakeUpCause() {
    // Get the wake-up reason from ESP32
    esp_sleep_wakeup_cause_t wakeUpCause = esp_sleep_get_wakeup_cause();

    // Determine the cause of the wake-up
    switch (wakeUpCause) {
        case ESP_SLEEP_WAKEUP_UNDEFINED:
            return -1;  // Undefined wakeup cause
        case ESP_SLEEP_WAKEUP_ALL:
            return -1;  // Not a wakeup cause, used to disable all wakeup sources
        case ESP_SLEEP_WAKEUP_EXT0:
        case ESP_SLEEP_WAKEUP_EXT1:
            return 1;  // GPIO wakeup (external signal)
        case ESP_SLEEP_WAKEUP_TIMER:
            return 0;  // Timer wakeup
        case ESP_SLEEP_WAKEUP_TOUCHPAD:
            return 2;  // Touchpad wakeup
        case ESP_SLEEP_WAKEUP_ULP:
            return 3;  // ULP program wakeup
        case ESP_SLEEP_WAKEUP_GPIO:
            return 4;  // GPIO wakeup (light sleep only)
        case ESP_SLEEP_WAKEUP_UART:
            return 5;  // UART wakeup (light sleep only)
        case ESP_SLEEP_WAKEUP_WIFI:
            return 6;  // WiFi wakeup (light sleep only)
        case ESP_SLEEP_WAKEUP_COCPU:
            return 7;  // COCPU wakeup
        case ESP_SLEEP_WAKEUP_COCPU_TRAP_TRIG:
            return 8;  // COCPU trap trigger wakeup
        case ESP_SLEEP_WAKEUP_BT:
            return 9;  // BT wakeup (light sleep only)
        default:
            return -1; // Unknown or undefined wake-up cause
    }
}
