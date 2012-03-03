#include <SPI.h>
#include <Ethernet.h>
#include <LiquidCrystal.h>

#include "cdp_listener.h"

#define VERSION_STR "v0.1"

#define LCD_INFO_NONE 0
#define LCD_INFO_CDP_DATA 1

#define printhex(n) {if((n)<0x10){Serial.print('0');}Serial.print((n),HEX);}

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

#define CHAR_SCROLLDOTS 1
byte charScrollDots[8] = {
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b10101,
  0b00000
};

#define CHAR_DELTA 2
byte charDelta[8] = {
  0b00000,
  0b00000,
  0b00100,
  0b01010,
  0b10001,
  0b11111,
  0b00000,
  0b00000
};

void setup() {
  // Init serial
  Serial.begin(115200);
  // Init LCD
  lcd.begin(20, 2);
  lcd.createChar(CHAR_SCROLLDOTS, charScrollDots);
  lcd.createChar(CHAR_DELTA, charDelta);
  
  // Let user know we're initializing
  Serial.println("Initializing");
  
  lcd.print("CDP Sniffino "VERSION_STR);
  lcd.setCursor(0, 1);
  lcd.print("Initializing");
  lcd.write(CHAR_SCROLLDOTS);
  
  cdp_listener_init();
  cdp_packet_handler = cdp_handler;
  
  // Let user know we're done initializing
  Serial.println("Initialization done");
  update_lcd();
}

volatile unsigned long last_cdp_received = 0;

volatile unsigned int lcd_delta_t = 0;
volatile unsigned int lcd_ttl = 0;

volatile unsigned int lcd_display_type = LCD_INFO_NONE;
volatile char* lcd_data_value;

void loop() {
  switch(cdp_listener_update()) {
    case CDP_STATUS_OK: break;
    case CDP_INCOMPLETE_PACKET: Serial.println(F("Incomplete packet received")); break;
    case CDP_UNKNOWN_LLC: Serial.println(F("Unexpected LLC packet")); break;
  }
  
  lcd_delta_t = (millis() - last_cdp_received)/1000;
}

void update_lcd() {
  unsigned int delta_t = lcd_delta_t;
  unsigned int ttl = lcd_ttl;
  
  lcd.clear();
  
  lcd.write(CHAR_DELTA);
  lcd.print(' ');
  if(delta_t < 10) lcd.print(' ');
  if(delta_t < 100) lcd.print(' ');
  lcd.print(delta_t);
  
  lcd.print("   TTL: ");
  if(ttl < 10) lcd.print(' ');
  if(ttl < 100) lcd.print(' ');
  lcd.print(ttl);

  lcd.setCursor(0, 1);
  switch(lcd_display_type) {
    case LCD_INFO_NONE: break;
    case LCD_INFO_CDP_DATA:
      lcd.print((const char*)lcd_data_value);
      break;
  }
  //TODO: print current item(s)
}

void cdp_handler(const byte cdpData[], size_t cdpDataIndex, size_t cdpDataLength, const byte macFrom[], size_t macLength) {
    unsigned long secs = millis()/1000;
    int min = secs / 60;
    int sec = secs % 60;
    Serial.print(min); Serial.print(':');
    if(sec < 10) Serial.print('0');
    Serial.print(sec);
    Serial.println();
    
    Serial.print(F("CDP packet received from "));
    print_mac(macFrom, 0, macLength);
    Serial.println();

    int cdpVersion = cdpData[cdpDataIndex++];
    if(cdpVersion != 0x02) {
      Serial.print(F("Version: "));
      Serial.println(cdpVersion);
    }
    
    int cdpTtl = cdpData[cdpDataIndex++];
    Serial.print(F("TTL: "));
    Serial.println(cdpTtl);
    
    lcd_ttl = cdpTtl;
    
    unsigned int cdpChecksum = (cdpData[cdpDataIndex] << 8) | cdpData[cdpDataIndex+1];
    cdpDataIndex += 2;
    Serial.print(F("Checksum: "));
    printhex(cdpChecksum >> 8);
    printhex(cdpChecksum & 0xFF);
    Serial.println();
    
    while(cdpDataIndex < cdpDataLength) { // read all remaining TLV fields
      unsigned int cdpFieldType = (cdpData[cdpDataIndex] << 8) | cdpData[cdpDataIndex+1];
      cdpDataIndex+=2;
      unsigned int cdpFieldLength = (cdpData[cdpDataIndex] << 8) | cdpData[cdpDataIndex+1];
      cdpDataIndex+=2;
      cdpFieldLength -= 4;
      
      switch(cdpFieldType) {
        case 0x0001:
          handleCdpAsciiField(F("Device ID: "), cdpData, cdpDataIndex, cdpFieldLength);
          break;
        case 0x00002:
          handleCdpAddresses(cdpData, cdpDataIndex, cdpFieldLength);
          break;
        case 0x0003:
          handleCdpAsciiField(F("Port ID: "), cdpData, cdpDataIndex, cdpFieldLength);
          break;
//          case 0x0004:
//            handleCdpCapabilities(cdpData, cdpDataIndex, cdpFieldLength);
//            break;
        case 0x0005:
          handleCdpAsciiField(F("Software Version: "), cdpData, cdpDataIndex, cdpFieldLength);
          break;
        case 0x0006:
          handleCdpAsciiField(F("Platform: "), cdpData, cdpDataIndex, cdpFieldLength);
          break;
        case 0x000a:
          handleCdpNumField(F("Native VLAN: "), cdpData, cdpDataIndex, cdpFieldLength);
          break;
        case 0x000b:
          handleCdpDuplex(cdpData, cdpDataIndex, cdpFieldLength);
          break;
        default:
          // TODO: raw field
//            Serial.print(F("Field "));
//            printhex(cdpFieldType >> 8); printhex(cdpFieldType & 0xFF);
//            Serial.print(F(", Length: "));
//            Serial.print(cdpFieldLength, DEC);
//            Serial.println();
          break;
      }
      
      cdpDataIndex += cdpFieldLength;
    }
    
    Serial.println();
  
}

void handleCdpAsciiField(const __FlashStringHelper * title, const byte a[], unsigned int offset, unsigned int length) {
  Serial.print(title);
  print_str(a, offset, length);
  Serial.println();
}

void handleCdpNumField(const __FlashStringHelper * title, const byte a[], unsigned int offset, unsigned int length) {
  unsigned long num = 0;
  for(unsigned int i=0; i<length; ++i) {
    num <<= 8;
    num += a[offset + i];
  }

  Serial.print(title);
  Serial.print(num, DEC);
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

void handleCdpDuplex(const byte a[], unsigned int offset, unsigned int length) {
  Serial.print(F("Duplex: "));
  if(a[offset]) {
    Serial.println(F("Full"));
  } else {
    Serial.println(F("Half"));
  }
}

void print_str(const byte a[], unsigned int offset, unsigned int length) {
  for(unsigned int i=offset; i<offset+length; ++i) {
    Serial.write(a[i]);
  }
}

void print_ip(const byte a[], unsigned int offset, unsigned int length) {
  for(unsigned int i=offset; i<offset+length; ++i) {
    if(i>offset) Serial.print('.');
    Serial.print(a[i], DEC);
  }
}

void print_mac(const byte a[], unsigned int offset, unsigned int length) {
  for(unsigned int i=offset; i<offset+length; ++i) {
    if(i>offset) Serial.print(':');
    if(a[i] < 0x10) Serial.print('0');
    Serial.print(a[i], HEX);
  }
}
