#include "wifi.h"
#include "display/display.h"
#include "secrets.h"
#include <WiFi.h>
#include <ArduinoOTA.h>
#include <Arduino.h>

static bool isConnected    = false;
static bool timeConfigured = false;

// Scan state
static bool     scanInProgress  = false;
static uint16_t scanResultCount = 0;

// ─── Connection ──────────────────────────────────────────────────────────────

bool getIsConnected() {
    return isConnected;
}

void initWiFi() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    Serial.println("[WiFi] Connecting in background...");
}

void handleWiFi() {
    wl_status_t status = WiFi.status();

    if (status == WL_CONNECTED) {
        if (!isConnected) {
            ArduinoOTA.begin();
            isConnected = true;
            Serial.println("[WiFi] Connected! RSSI: " + String(WiFi.RSSI()));
            drawHomeMenu();
        }

        if (!timeConfigured) {
            configTime(2 * 3600, 3600, "pool.ntp.org", "time.nist.gov");
            timeConfigured = true;
            Serial.println("[WiFi] NTP sync started");
        }

        ArduinoOTA.handle();

    } else {
        if (isConnected) {
            isConnected    = false;
            timeConfigured = false;
            Serial.println("[WiFi] Disconnected");
            drawHomeMenu();
        }
    }
}

// ─── Scanning ─────────────────────────────────────────────────────────────────

void startScan() {
    if (scanInProgress) {
        Serial.println("[WiFi] Scan already in progress");
        return;
    }

    scanInProgress  = true;
    scanResultCount = 0;

    // Async scan, include hidden networks
    WiFi.scanNetworks(true, true);
    Serial.println("[WiFi] Scan started...");
}

void stopScan() {
    scanInProgress = false;
    WiFi.scanDelete();
    Serial.println("[WiFi] Scan stopped");
}

bool isScanInProgress() {
    return scanInProgress;
}

uint16_t getScanResultCount() {
    return scanResultCount;
}

uint16_t getScanResults(NetworkInfo* results, uint16_t max_results) {
    if (!results) return 0;

    int16_t scan_count = WiFi.scanComplete();

    // Still running
    if (scan_count == WIFI_SCAN_RUNNING) {
        return 0;
    }

    // Error or nothing found
    if (scan_count <= 0) {
        scanInProgress = false;
        Serial.println("[WiFi] No networks found or scan error");
        return 0;
    }

    uint16_t count = min((uint16_t)scan_count, max_results);

    for (uint16_t i = 0; i < count; i++) {
        memset(&results[i], 0, sizeof(NetworkInfo));

        // SSID
        String ssid = WiFi.SSID(i);
        strncpy(results[i].ssid, ssid.c_str(), 32);
        results[i].ssid[32] = '\0';

        // BSSID
        uint8_t* bssid = WiFi.BSSID(i);
        if (bssid) memcpy(results[i].bssid, bssid, 6);

        // Signal, channel, auth
        results[i].rssi      = WiFi.RSSI(i);
        results[i].channel   = WiFi.channel(i);
        results[i].auth_mode = (uint8_t)WiFi.encryptionType(i);

        Serial.printf("[WiFi] %d: %-32s ch%-2d %ddBm %s\n",
            i + 1,
            results[i].ssid[0] ? results[i].ssid : "[Hidden]",
            results[i].channel,
            results[i].rssi,
            results[i].auth_mode == 0 ? "Open" : "Secured"
        );
    }

    scanResultCount = count;
    scanInProgress  = false;

    Serial.printf("[WiFi] Scan complete — %d networks found\n", count);
    return count;
}