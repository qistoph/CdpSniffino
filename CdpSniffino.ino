#include <SPI.h>
#include <Ethernet.h>
#include <utility/w5100.h>

SOCKET s; // the socket that will be openend in RAW mode
byte rbuf[1500+14]; //receive buffer
//byte sbuf[1500+14]; //send buffer
int rbuflen; // length of data to receive
//int sbuflen; // length of data to send

byte cdp_mac[] = {0x01, 0x00, 0x0c, 0xcc, 0xcc, 0xcc};
byte llc_bytes[] = {0xaa, 0xaa, 0x03, 0x00, 0x00, 0x0c, 0x20, 0x00};

#define printhex(n) {if((n)<0x10){Serial.print('0');}Serial.print((n),HEX);}

void setup() {
  Serial.begin(115200);
  Serial.println("Hello, initializing");
  
  // initialize the w5100 chip and open a RAW socket
  W5100.init();
  W5100.writeSnMR(s, SnMR::MACRAW);
  W5100.execCmdSn(s, Sock_OPEN);
  
  Serial.println("Initialization done");
}

void loop() {
  
  // check if we have received something
  rbuflen = W5100.getRXReceivedSize(s);
//  
  if(rbuflen>0) {
    
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
      
      unsigned long secs = millis()/1000;
      int min = secs / 60;
      int sec = secs % 60;
      Serial.print(min); Serial.print(':');
      if(sec < 10) Serial.print('0');
      Serial.print(sec);
      Serial.println();

      Serial.print(F("CDP packet received from "));
      print_mac(rbuf, rbufIndex, 6);
      Serial.println();
      rbufIndex += 6;
      
      unsigned int packet_length = (rbuf[rbufIndex] << 8) | rbuf[rbufIndex+1];
      rbufIndex+=2;
      
      if(packet_length != rbuflen - rbufIndex) {
        Serial.println(F("Incomplete packet received"));
        //todo: abort?
      }
      
      // expecting LLC bytes
      if(!byte_array_contains(rbuf, rbufIndex, llc_bytes, sizeof(llc_bytes))) {
        Serial.println(F("Unexpected LLC packet"));
      }
      rbufIndex += sizeof(llc_bytes);
      
      int cdpVersion = rbuf[rbufIndex++];
      Serial.print(F("Version: "));
      Serial.println(cdpVersion);
      if(cdpVersion != 0x02) {
        Serial.print(F("This tool implements CDP version 2, not "));
        Serial.println(cdpVersion);
      }
      
      int cdpTtl = rbuf[rbufIndex++];
      Serial.print(F("TTL: "));
      Serial.println(cdpTtl);
      
      unsigned int cdpChecksum = (rbuf[rbufIndex] << 8) | rbuf[rbufIndex+1];
      rbufIndex += 2;
      Serial.print(F("Checksum: "));
      printhex(cdpChecksum >> 8);
      printhex(cdpChecksum & 0xFF);
      Serial.println();
      
      while(rbufIndex < rbuflen) { // read all remaining TLV fields
        unsigned int cdpFieldType = (rbuf[rbufIndex] << 8) | rbuf[rbufIndex+1];
        rbufIndex+=2;
        unsigned int cdpFieldLength = (rbuf[rbufIndex] << 8) | rbuf[rbufIndex+1];
        rbufIndex+=2;
        
        switch(cdpFieldType) {
          case 0x0001:
            handleCdpAsciiField(F("Device ID: "), rbuf, rbufIndex, cdpFieldLength);
            break;
          case 0x00002:
            handleCdpAddresses(rbuf, rbufIndex, cdpFieldLength);
            break;
          case 0x0003:
            handleCdpAsciiField(F("Port ID: "), rbuf, rbufIndex, cdpFieldLength);
            break;
//          case 0x0004:
//            handleCdpCapabilities(rbuf, rbufIndex, cdpFieldLength);
//            break;
          case 0x0005:
            handleCdpAsciiField(F("Software Version: "), rbuf, rbufIndex, cdpFieldLength);
            break;
          case 0x0006:
            handleCdpAsciiField(F("Platform: "), rbuf, rbufIndex, cdpFieldLength);
            break;
          default:
            Serial.print(F("Field "));
            printhex(cdpFieldType >> 8); printhex(cdpFieldType & 0xFF);
            Serial.print(F(", Length: "));
            Serial.print(cdpFieldLength, DEC);
            Serial.println();
            break;
            //todo: other types
        }
        
        rbufIndex += cdpFieldLength - 4;
      }
      
      Serial.println();
    }
  }
}

void handleCdpAsciiField(const __FlashStringHelper * title, const byte a[], unsigned int offset, unsigned int length) {
  Serial.print(title);
  print_str(a, offset, length);
  Serial.println();
}

void handleCdpAddresses(const byte a[], unsigned int offset, unsigned int length) {
  Serial.println(F("Addresses: "));
  unsigned long numOfAddrs = (a[offset] << 24) | (a[offset+1] << 16) | (a[offset+2] << 8) | a[offset+3];
  offset += 4;
  for(unsigned long i=0; i<numOfAddrs; ++i) {
    unsigned int protoType = a[offset++];
    unsigned int protoLength = a[offset++];
    byte proto[8];
    for(unsigned int j=0; j<protoLength; ++j) {
      proto[j] = a[offset++];
    }
    unsigned int addressLength = (a[offset] << 8) | a[offset+1];
    offset += 2;
    byte address[4];
    if(addressLength != 4) Serial.println(F("Expecting address length: 4"));
    for(unsigned int j=0; j<addressLength; ++j) {
      address[j] = a[offset++];
    }
    
    Serial.print("- ");
    print_ip(address, 0, 4);
  }
  Serial.println();
}

void print_str(const byte a[], unsigned int offset, unsigned int length) {
  for(int i=offset; i<offset+length; ++i) {
    Serial.write(a[i]);
  }
}

void print_ip(const byte a[], unsigned int offset, unsigned int length) {
  for(int i=offset; i<offset+length; ++i) {
    if(i>offset) Serial.print('.');
    Serial.print(a[i], DEC);
  }
}

void print_mac(const byte a[], unsigned int offset, unsigned int length) {
  for(int i=offset; i<offset+length; ++i) {
    if(i>offset) Serial.print(':');
    if(a[i] < 0x10) Serial.print('0');
    Serial.print(a[i], HEX);
  }
}

bool byte_array_contains(const byte a[], unsigned int offset, const byte b[], unsigned int length) {
  for(int i=offset, j=0; j<length; ++i, ++j) {
    if(a[i] != b[j]) return false;
  }
  
  return true;
}
