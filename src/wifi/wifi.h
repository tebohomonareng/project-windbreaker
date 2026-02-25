#pragma once
#include <stdint.h>

struct NetworkInfo {
    char     ssid[33];
    uint8_t  bssid[6];
    int32_t  rssi;
    uint8_t  channel;
    uint8_t  auth_mode;
};

// Connection
void initWiFi();
void handleWiFi();
bool getIsConnected();

// Scanning
void     startScan();
void     stopScan();
bool     isScanInProgress();
uint16_t getScanResults(NetworkInfo* results, uint16_t max_results);
uint16_t getScanResultCount();