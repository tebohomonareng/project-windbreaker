#include "joystick.h"
#include "config.h"
#include "ui/menu.h"
#include <Arduino.h>

static int selectedMenuItem = 0;
static unsigned long lastJoystickRead = 0;
static bool lastButtonState = true;
static int joystickX = 2048;
static int joystickY = 2048;
static int lastDirectionX = 0;  // Track previous direction to detect changes
static int lastDirectionY = 0;

void initJoystick() {
    pinMode(JOYSTICK_SW, INPUT_PULLUP);
    analogSetPinAttenuation(JOYSTICK_VRX, ADC_11db);
    analogSetPinAttenuation(JOYSTICK_VRY, ADC_11db);
}

int getSelectedMenuItem() {
    return selectedMenuItem;
}

void handleJoystickInput() {
    unsigned long currentTime = millis();

    int vrx = analogRead(JOYSTICK_VRX);
    int vry = analogRead(JOYSTICK_VRY);
    bool buttonPressed = digitalRead(JOYSTICK_SW) == LOW;

    // Convert to offset from center
    joystickX = vrx - 2048;
    joystickY = vry - 2048;

    // Determine current direction (neutral, up, down, left, right)
    int currentDirectionX = 0;
    int currentDirectionY = 0;

    if (joystickX < -JOYSTICK_THRESHOLD) {
        currentDirectionX = -1;  // Left
    } else if (joystickX > JOYSTICK_THRESHOLD) {
        currentDirectionX = 1;   // Right
    }

    if (joystickY < -JOYSTICK_THRESHOLD) {
        currentDirectionY = -1;  // Up
    } else if (joystickY > JOYSTICK_THRESHOLD) {
        currentDirectionY = 1;   // Down
    }

    // Only send input if direction changed AND within debounce window
    if ((currentTime - lastJoystickRead) >= JOYSTICK_DEBOUNCE_MS) {
        if (currentDirectionX != lastDirectionX || currentDirectionY != lastDirectionY) {
            handleMenuInput(joystickX, joystickY, false);
            lastJoystickRead = currentTime;
            lastDirectionX = currentDirectionX;
            lastDirectionY = currentDirectionY;
        }
    }

    // Button press
    if (buttonPressed && lastButtonState) {
        lastButtonState = false;
        handleMenuInput(joystickX, joystickY, true);
        Serial.println("Button pressed");
    }
    else if (!buttonPressed && !lastButtonState) {
        lastButtonState = true;
    }
}