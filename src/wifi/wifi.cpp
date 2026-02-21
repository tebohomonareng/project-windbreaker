#include "wifi.h"
#include "display/display.h"
#include "secrets.h"
#include <WiFi.h>
#include <ArduinoOTA.h>
#include <Arduino.h>

static bool isConnected = false;
static bool timeConfigured = false;

bool getIsConnected() {
    return isConnected;
}

void initWiFi() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    Serial.println("WiFi connecting in background...");
}

void handleWiFi() {
    wl_status_t status = WiFi.status();

    if (status == WL_CONNECTED) {
        if (!isConnected) {
            ArduinoOTA.begin();
            isConnected = true;
            Serial.println("WiFi Connected! RSSI: " + String(WiFi.RSSI()));
            drawHomeScreen();
        }

        if (!timeConfigured) {
            configTime(2 * 3600, 3600, "pool.ntp.org", "time.nist.gov");
            timeConfigured = true;
            Serial.println("NTP sync started");
        }

        ArduinoOTA.handle();

    } else {
        if (isConnected) {
            isConnected = false;
            timeConfigured = false;
            Serial.println("WiFi disconnected");
            drawHomeScreen();
        }
    }
}