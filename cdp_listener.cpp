#include "cdp_listener.h"

void (*cdp_packet_handler)(const byte* cdpData, size_t cdpDataIndex, size_t cdpDataLength, const byte* macFrom, size_t macLength);

SOCKET s; // the socket that will be openend in RAW mode
byte rbuf[500+14]; //receive buffer (was 1500+14, but costs way too much SRAM)
int rbuflen; // length of data to receive

#define MAC_LENGTH 6

byte cdp_mac[] = {0x01, 0x00, 0x0c, 0xcc, 0xcc, 0xcc};
byte llc_bytes[] = {0xaa, 0xaa, 0x03, 0x00, 0x00, 0x0c, 0x20, 0x00};

void cdp_listener_init() {
  // initialize the w5100 chip and open a RAW socket
  W5100.init();
  W5100.writeSnMR(s, SnMR::MACRAW);
  W5100.execCmdSn(s, Sock_OPEN);
}

CDP_STATUS cdp_listener_update() {
  // check if we have received something
  rbuflen = W5100.getRXReceivedSize(s);
  
  if(rbuflen>0) {
    if(rbuflen > sizeof(rbuf))
      rbuflen = sizeof(rbuf);
    W5100.recv_data_processing(s, rbuf, rbuflen);
    W5100.execCmdSn(s, Sock_RECV);
    
//    Serial.print(F("Received "));
//    Serial.println(rbuflen, DEC);
    
//    for(int i=0; i<rbuflen-2; ++i) {
//      Serial.print(rbuf[i+2], HEX);
//      Serial.print(' ');
//      if(i%16==15) Serial.println();
//    }
//    Serial.println();
  
    unsigned int rbufIndex = 2;
    if(byte_array_contains(rbuf, rbufIndex, cdp_mac, sizeof(cdp_mac))) {
      rbufIndex += sizeof(cdp_mac);
      
      byte* macFrom = rbuf+rbufIndex;
      rbufIndex += MAC_LENGTH; // received from, MAC address = 6 bytes
      
      unsigned int packet_length = (rbuf[rbufIndex] << 8) | rbuf[rbufIndex+1];
      rbufIndex+=2;
//      
//      Serial.print(packet_length);
//      Serial.print(F(" vs "));
//      Serial.print(rbuflen);
//      Serial.print(F(" - "));
//      Serial.println(rbufIndex);
      if(packet_length != rbuflen - rbufIndex) {
        return CDP_INCOMPLETE_PACKET;
      }
      
      // expecting LLC bytes
      if(!byte_array_contains(rbuf, rbufIndex, llc_bytes, sizeof(llc_bytes))) {
        return CDP_UNKNOWN_LLC;
      }
      rbufIndex += sizeof(llc_bytes);
      
      if(cdp_packet_handler != NULL) {
        cdp_packet_handler(rbuf, rbufIndex, rbuflen - rbufIndex, macFrom, MAC_LENGTH);
      }
    }
  }
  
  return CDP_STATUS_OK;
}

bool byte_array_contains(const byte a[], unsigned int offset, const byte b[], unsigned int length) {
  for(unsigned int i=offset, j=0; j<length; ++i, ++j) {
    if(a[i] != b[j]) return false;
  }
  
  return true;
}
