#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoOTA.h>
#include "secrets.h"

// --- SETTINGS ---
const char* ssid = "";
const char* password = "";

const int LED_PIN = 2; // Onboard LED for FireBeetle 2 ESP32-E

// Timing variables for non-blocking blinks
unsigned long previousMillis = 0;
int blinkInterval = 1000; // Default slow blink (1 second)

void setup() {
    Serial.begin(115200);
    pinMode(LED_PIN, OUTPUT);

    Serial.println("\n--- Starting WiFi Connection ---");
    WiFi.begin(WIFI_SSID, WIFI_PASS);
}

void loop() {
    unsigned long currentMillis = millis();
    wl_status_t status = WiFi.status();

    if (status == WL_CONNECTED) {
        // 1. CONNECTED: LED Solid ON
        digitalWrite(LED_PIN, HIGH);
        
        // Start OTA only once when connected
        static bool otaStarted = false;
        if (!otaStarted) {
            ArduinoOTA.begin();
            Serial.print("Connected! IP: ");
            Serial.println(WiFi.localIP());
            otaStarted = true;
        }
        ArduinoOTA.handle();

    } else if (status == WL_DISCONNECTED || status == WL_IDLE_STATUS) {
        // 2. ATTEMPTING: Medium Blink (500ms)
        blinkInterval = 500;
        if (currentMillis - previousMillis >= blinkInterval) {
            previousMillis = currentMillis;
            digitalWrite(LED_PIN, !digitalRead(LED_PIN));
        }
    } else {
        // 3. NO WIFI / ERROR: Slow Blink (1500ms)
        blinkInterval = 1500;
        if (currentMillis - previousMillis >= blinkInterval) {
            previousMillis = currentMillis;
            digitalWrite(LED_PIN, !digitalRead(LED_PIN));
        }
    }
}