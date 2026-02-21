#include "joystick.h"
#include "config.h"
#include <Arduino.h>

static int selectedMenuItem = 0;
static unsigned long lastJoystickRead = 0;
static bool lastButtonState = true;

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

    int vry = analogRead(JOYSTICK_VRY);
    bool buttonPressed = digitalRead(JOYSTICK_SW) == LOW;

    if (vry < JOYSTICK_THRESHOLD) {
        if (currentTime - lastJoystickRead >= JOYSTICK_DEBOUNCE_MS) {
            selectedMenuItem = (selectedMenuItem + 1) % MENU_ITEM_COUNT;
            lastJoystickRead = currentTime;
            Serial.println("Joystick Down - Selected: " + String(selectedMenuItem));
        }
    }
    else if (vry > 4095 - JOYSTICK_THRESHOLD) {
        if (currentTime - lastJoystickRead >= JOYSTICK_DEBOUNCE_MS) {
            selectedMenuItem = (selectedMenuItem - 1 + MENU_ITEM_COUNT) % MENU_ITEM_COUNT;
            lastJoystickRead = currentTime;
            Serial.println("Joystick Up - Selected: " + String(selectedMenuItem));
        }
    }

    if (buttonPressed && lastButtonState) {
        lastButtonState = false;
        Serial.println("Button pressed on menu item: " + String(selectedMenuItem));
        switch (selectedMenuItem) {
            case 0: Serial.println("Selected: WiFi"); break;
            case 1: Serial.println("Selected: BT Scan"); break;
            case 2: Serial.println("Selected: Settings"); break;
        }
    }
    else if (!buttonPressed && !lastButtonState) {
        lastButtonState = true;
    }
}