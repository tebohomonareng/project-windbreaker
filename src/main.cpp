#include <Arduino.h>
#include "config.h"
#include "display/display.h"
#include "joystick/joystick.h"
#include "wifi/wifi.h"
#include "attack/attack.h"
#include "attack/sniffer.h"
#include "ui/menu.h"
#include <WiFi.h> 

unsigned long previousMillis = 0;
unsigned long lastScreenUpdate = 0;

void setup() {
    Serial.begin(115200);
    pinMode(LED_PIN, OUTPUT);

    initDisplay();
    showLogo();
    initJoystick();
    initWiFi();
    initAttackManager();
    initSniffer();
    initMenuSystem();
}

void loop() {
    unsigned long currentMillis = millis();

    handleJoystickInput();
    handleWiFi();
    handleAttackLoop();

    // LED blink when disconnected
    if (!getIsConnected()) {
        int blinkInterval = (WiFi.status() == WL_DISCONNECTED || 
                             WiFi.status() == WL_IDLE_STATUS) ? 500 : 1500;
        if (currentMillis - previousMillis >= blinkInterval) {
            previousMillis = currentMillis;
            digitalWrite(LED_PIN, !digitalRead(LED_PIN));
        }
    } else {
        digitalWrite(LED_PIN, HIGH);
    }

    // Screen refresh
    int refreshRate = getIsConnected() ? 1000 : 2000;
    if (currentMillis - lastScreenUpdate >= refreshRate) {
        lastScreenUpdate = currentMillis;
        drawCurrentMenu();
    }
}