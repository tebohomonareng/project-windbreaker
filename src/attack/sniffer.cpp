#include "sniffer.h"
#include <esp_wifi.h>
#include <Arduino.h>
#include <WiFi.h>

static volatile uint32_t beaconCount = 0;
static volatile uint32_t probeReqCount = 0;
static volatile uint32_t probeRespCount = 0;
static volatile uint32_t authCount = 0;
static volatile uint32_t assocReqCount = 0;
static volatile uint32_t assocRespCount = 0;
static volatile uint32_t deauthCount = 0;
static volatile uint32_t dataCount = 0;
static volatile uint32_t otherCount = 0;

static bool snifferEnabled = false;
static uint8_t snifferChannel = 0;

// Packet frame types
#define FRAME_TYPE_MGMT     0x00
#define FRAME_SUBTYPE_BEACON 0x08
#define FRAME_SUBTYPE_PROBE_REQ 0x04
#define FRAME_SUBTYPE_PROBE_RESP 0x05
#define FRAME_SUBTYPE_AUTH 0x0B
#define FRAME_SUBTYPE_ASSOC_REQ 0x00
#define FRAME_SUBTYPE_ASSOC_RESP 0x01
#define FRAME_SUBTYPE_DEAUTH 0x0C

typedef struct {
    unsigned protocol : 2;
    unsigned type : 2;
    unsigned subtype : 4;
    unsigned to_ds : 1;
    unsigned from_ds : 1;
    unsigned more_frag : 1;
    unsigned retry : 1;
    unsigned pwr_mgt : 1;
    unsigned more_data : 1;
    unsigned protected_frame : 1;
    unsigned order : 1;
} FrameControl;

// Callback for packet promiscuous mode
static void handlePacket(void* buf, wifi_promiscuous_pkt_type_t type) {
    if (type != WIFI_PKT_MGMT) {
        return;
    }
    
    wifi_promiscuous_pkt_t* packet = (wifi_promiscuous_pkt_t*)buf;
    wifi_pkt_rx_ctrl_t* rx_ctrl = &packet->rx_ctrl;
    uint8_t* payload = packet->payload;
    
    if (rx_ctrl->sig_len < 24) {
        return; // Too short for a valid frame
    }
    
    FrameControl* frame_control = (FrameControl*)payload;
    
    uint8_t xx = frame_control->type;
    uint8_t subtype = frame_control->subtype;
    
    if (xx == FRAME_TYPE_MGMT) {
        switch (subtype) {
            case FRAME_SUBTYPE_BEACON:
                beaconCount++;
                break;
            case FRAME_SUBTYPE_PROBE_REQ:
                probeReqCount++;
                break;
            case FRAME_SUBTYPE_PROBE_RESP:
                probeRespCount++;
                break;
            case FRAME_SUBTYPE_AUTH:
                authCount++;
                break;
            case FRAME_SUBTYPE_ASSOC_REQ:
                assocReqCount++;
                break;
            case FRAME_SUBTYPE_ASSOC_RESP:
                assocRespCount++;
                break;
            case FRAME_SUBTYPE_DEAUTH:
                deauthCount++;
                break;
            default:
                otherCount++;
                break;
        }
    } else {
        dataCount++;
    }
}

void initSniffer() {
    Serial.println("[SNIFFER] Packet sniffer initialized");
}

void enablePacketSniffer(uint8_t channel) {
    if (snifferEnabled) {
        return;
    }
    
    snifferChannel = channel;
    resetPacketCounts();
    
    // Set WiFi to promiscuous mode
    WiFi.mode(WIFI_AP);
    esp_wifi_set_promiscuous(true);
    esp_wifi_set_promiscuous_rx_cb(handlePacket);
    esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
    
    snifferEnabled = true;
    Serial.printf("[SNIFFER] Packet sniffer enabled on channel %d\n", channel);
}

void disablePacketSniffer() {
    if (!snifferEnabled) {
        return;
    }
    
    esp_wifi_set_promiscuous(false);
    snifferEnabled = false;
    
    Serial.println("[SNIFFER] Packet sniffer disabled");
}

uint32_t getPacketCount(PacketType type) {
    switch (type) {
        case PACKET_TYPE_BEACON:
            return beaconCount;
        case PACKET_TYPE_PROBE_REQ:
            return probeReqCount;
        case PACKET_TYPE_PROBE_RESP:
            return probeRespCount;
        case PACKET_TYPE_AUTH:
            return authCount;
        case PACKET_TYPE_ASSOC_REQ:
            return assocReqCount;
        case PACKET_TYPE_ASSOC_RESP:
            return assocRespCount;
        case PACKET_TYPE_DEAUTH:
            return deauthCount;
        case PACKET_TYPE_DATA:
            return dataCount;
        default:
            return otherCount;
    }
}

uint32_t getTotalPacketCount() {
    return beaconCount + probeReqCount + probeRespCount + authCount +
           assocReqCount + assocRespCount + deauthCount + dataCount + otherCount;
}

void resetPacketCounts() {
    beaconCount = 0;
    probeReqCount = 0;
    probeRespCount = 0;
    authCount = 0;
    assocReqCount = 0;
    assocRespCount = 0;
    deauthCount = 0;
    dataCount = 0;
    otherCount = 0;
}
