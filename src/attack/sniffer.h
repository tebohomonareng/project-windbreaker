#pragma once

#include <stdint.h>

typedef struct {
    uint8_t src_mac[6];
    uint8_t dst_mac[6];
    uint16_t packet_type;
    int8_t rssi;
    uint8_t channel;
    uint32_t timestamp;
    uint16_t length;
} PacketInfo;

typedef enum {
    PACKET_TYPE_BEACON = 0,
    PACKET_TYPE_PROBE_REQ = 1,
    PACKET_TYPE_PROBE_RESP = 2,
    PACKET_TYPE_AUTH = 3,
    PACKET_TYPE_ASSOC_REQ = 4,
    PACKET_TYPE_ASSOC_RESP = 5,
    PACKET_TYPE_DEAUTH = 6,
    PACKET_TYPE_DATA = 7,
    PACKET_TYPE_OTHER = 8
} PacketType;

void initSniffer();
void enablePacketSniffer(uint8_t channel);
void disablePacketSniffer();
uint32_t getPacketCount(PacketType type);
uint32_t getTotalPacketCount();
void resetPacketCounts();
