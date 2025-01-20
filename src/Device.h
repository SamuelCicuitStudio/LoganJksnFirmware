#ifndef DEVICE_H
#define DEVICE_H

#include "ConfigManager.h"  // Include the config file for pin definitions


class Device {
public:
    // Constructor
    Device();
    // Initialize GPIOs
    void begin();
    // Blink the LED with a given interval (in milliseconds)
    void blinkLED(unsigned long interval);
    // Check if the button is pressed
    bool isButtonPressed();
    // Turn the buzzer on or off
    void controlBuzzer(bool state);
    void deepSleep(unsigned long sleepDuration);
    int getWakeUpCause();

private:
    unsigned long _lastBlinkTime;
    bool _ledState;
};

#endif
