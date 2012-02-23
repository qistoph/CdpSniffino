#ifndef _CDP_LISTENER_H_
#define _CDP_LISTENER_H_

#include <Arduino.h>

#include <SPI.h>
#include <Ethernet.h>
#include <utility/w5100.h>

typedef uint8_t CDP_STATUS;
#define CDP_STATUS_OK 0
#define CDP_INCOMPLETE_PACKET 1
#define CDP_UNKNOWN_LLC 2

// Public functions
void cdp_listener_init();
CDP_STATUS cdp_listener_update();

// Public handlers
extern void (*cdp_packet_handler)(const byte* cdpData, size_t cdpDataIndex, size_t cdpDataLength, const byte* macFrom, size_t macLength);

// Internal functions
void cdp_packet_received(const byte a[], unsigned int offset, unsigned int length);

// Helper functions
bool byte_array_contains(const byte a[], unsigned int offset, const byte b[], unsigned int length);

#endif
