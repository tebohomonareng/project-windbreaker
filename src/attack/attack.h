#pragma once

#include <stdint.h>

// Attack types
typedef enum {
    ATTACK_NONE = 0,
    ATTACK_SCAN,
    ATTACK_DEAUTH,
    ATTACK_BEACON_SPOOF,
    ATTACK_SIGNAL_JAM
} AttackType;

// Network info structure
typedef struct {
    uint8_t bssid[6];
    char ssid[33];
    int8_t rssi;
    uint8_t channel;
    uint8_t auth_mode;
    uint16_t beacon_interval;
} NetworkInfo;

// Attack state
typedef enum {
    ATTACK_STATE_IDLE = 0,
    ATTACK_STATE_RUNNING,
    ATTACK_STATE_PAUSED,
    ATTACK_STATE_ERROR
} AttackState;

// Function declarations
void initAttackManager();
void startNetworkScan();
void stopNetworkScan();
uint16_t getScanResults(NetworkInfo* results, uint16_t max_results);

void startDeauthAttack(const uint8_t* target_bssid, uint8_t channel);
void stopDeauthAttack();
AttackState getDeauthState();
uint32_t getDeauthPacketsCount();

void startBeaconSpoof(const char* ssid, uint8_t channel);
void stopBeaconSpoof();
AttackState getBeaconState();

void handleAttackLoop();
AttackType getCurrentAttack();
void setCurrentAttack(AttackType attack);
