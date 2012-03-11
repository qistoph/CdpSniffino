#include <SPI.h>
#include <Ethernet.h>
#include <Wire.h>
#include <LiquidCrystal.h>
#include <DebounceButton.h>

#include "cdp_listener.h" // Uses digital pins 11, 12, 13 on Duemilanove
#include "lcd_info.h"

#define VERSION_STR "v0.1"

#define printhex(n) {if((n)<0x10){Serial.print('0');}Serial.print((n),HEX);}

// Configure your LCD pins here
// For instructions see http://arduino.cc/en/Tutorial/LiquidCrystal
// and http://arduino.cc/en/Reference/LiquidCrystal
LiquidCrystal lcd(8, 7, 6, 5, 4, 3, 2);

DebounceButton btnNext(9, DBTN_PULLUP_INTERNAL, 50, 800, 400); // 50 ms debounce, 0.8 sec before hold interval, 400 ms hold events
DebounceButton btnMore(10, DBTN_PULLUP_INTERNAL, 50, 800, 400); // 50 ms debounce, 0.8 sec before hold interval, 400 ms hold events

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
  
  // Let user know we're initializing
  Serial.println(F("Initializing"));
  
  btnNext.onPress = btnNext_press;
  btnNext.onHold = btnNext_hold;
  btnMore.onPress = btnMore_press;
  btnMore.onHold = btnMore_hold;
  
  lcd.createChar(CHAR_SCROLLDOTS, charScrollDots);
  lcd.createChar(CHAR_DELTA, charDelta);
  
  lcd.print("CDP Sniffino "VERSION_STR);
  lcd.setCursor(0, 1);
  lcd.print("Initializing");
  lcd.write(CHAR_SCROLLDOTS);
  
  cdp_listener_init();
  cdp_packet_handler = cdp_handler;
  
  // Let user know we're done initializing
  Serial.println(F("Initialization done"));
  lcd.clear();
  update_lcd();
}

volatile unsigned long last_cdp_received = 0;
volatile unsigned long cdp_packets_received = 0;

volatile unsigned int lcd_delta_t = 0;
volatile unsigned int lcd_ttl = 0;

unsigned long last_lcd_update = 0;
unsigned long last_serial_ping = 0;

void loop() {
  switch(cdp_listener_update()) {
    case CDP_STATUS_OK: break;
    case CDP_INCOMPLETE_PACKET: Serial.println(F("Incomplete packet received.")); break;
    case CDP_UNKNOWN_LLC: Serial.println(F("Unexpected LLC packet.")); break;
  }
  
  DebounceButton::updateAll();
  
  lcd_delta_t = (millis() - last_cdp_received)/1000;
  
  if(millis() > last_lcd_update + 100) {
    update_lcd();
    last_lcd_update = millis();
  }
  
  if(millis() > last_serial_ping + 2000) {
    Serial.println(lcd_delta_t);
    last_serial_ping = millis();
  }
}

void btnNext_press(DebounceButton* btn) {
  lcd_info_next();
}

void btnNext_hold(DebounceButton* btn) {
  lcd_info_next();
}

void btnMore_press(DebounceButton* btn) {
  lcd_info_more();
}

void btnMore_hold(DebounceButton* btn) {
  lcd_info_more();
}

void update_lcd() {
  unsigned int delta_t = lcd_delta_t;
  unsigned int ttl = lcd_ttl;
  unsigned int i;
  
  lcd.setCursor(0, 0);
  
  lcd.write(CHAR_DELTA);
  //lcd.print("t ");
  if(delta_t < 10) lcd.print(' ');
  if(delta_t < 100) lcd.print(' ');
  lcd.print(delta_t);
  
  lcd.print("/");
  if(ttl < 10) lcd.print(' ');
  if(ttl < 100) lcd.print(' ');
  lcd.print(ttl);
  
  lcd.print(" @ ");
  lcd.print(cdp_packets_received);
  if(cdp_packets_received < 10) lcd.print(' ');
  if(cdp_packets_received < 100) lcd.print(' ');

  lcd.setCursor(0, 1);
  menu_item* curr = &menu[menu_current];
  char lcd_data[21]; // 20 + \0

  if(curr != NULL && curr->visible) {
    size_t label_length = strlen(curr->label);
    strncpy(lcd_data, curr->label, 20);
    i = label_length;
    lcd_data[i++] = ':';
    lcd_data[i++] = ' ';

    if(lcd_more_offset > 0) lcd_data[i++] = CHAR_SCROLLDOTS;
    size_t value_length = strlen(curr->value);
    if((value_length - lcd_more_offset) > (20 - i)) {
      value_length = 20 - i - 1;
      lcd_data[19] = CHAR_SCROLLDOTS;
    }
    strncpy(&lcd_data[i], &curr->value[lcd_more_offset], value_length);
  } else {
    for(i=0; i<20; ++i) lcd_data[i] = ' ';
  }

  lcd_data[20] = '\0';
  boolean complete = 0;
  for(i = 0; i<20; ++i) {
    if(lcd_data[i] == '\0') complete = 1;
    if(complete)
      lcd.write((uint8_t)' ');
    else
      lcd.write(lcd_data[i]);
  }
//  printf("|%s|\n", lcd_data);
}

char value_mac_buffer[6*2 + 2 + 1]; // 6 bytes * 2 chars + 2 * : + 1 * \0
char value_ip_buffer[4*3 + 3 + 1]; // 3*4 chars  + 3 * . + 1 * \0
char value_devid_buffer[20 + 1]; // 20 chars + \0
char value_port_buffer[20 + 1]; // 20 chars + \0

void cdp_handler(const byte cdpData[], size_t cdpDataIndex, size_t cdpDataLength, const byte macFrom[], size_t macLength) {
  last_cdp_received = millis();
  cdp_packets_received++;
  
  unsigned long secs = millis()/1000;
  int min = secs / 60;
  int sec = secs % 60;
  Serial.print(min); Serial.print(':');
  if(sec < 10) Serial.print('0');
  Serial.print(sec);
  Serial.println();
  
  Serial.print(F("CDP packet received from "));
  set_mac(macFrom, 0, macLength);
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
        handleCdpAsciiField(F("Device ID: "), cdpData, cdpDataIndex, cdpFieldLength, value_devid_buffer, sizeof(value_devid_buffer));
        set_menu(LABEL_DEVICE_ID, value_devid_buffer);
        break;
      case 0x00002:
        handleCdpAddresses(cdpData, cdpDataIndex, cdpFieldLength);
        break;
      case 0x0003:
        handleCdpAsciiField(F("Port ID: "), cdpData, cdpDataIndex, cdpFieldLength, value_port_buffer, sizeof(value_port_buffer));
        set_menu(LABEL_PORT_ID, value_port_buffer);
        break;
//          case 0x0004:
//            handleCdpCapabilities(cdpData, cdpDataIndex, cdpFieldLength);
//            break;
      case 0x0005:
        //handleCdpAsciiField(F("Software Version: "), cdpData, cdpDataIndex, cdpFieldLength);
        break;
      case 0x0006:
        //handleCdpAsciiField(F("Platform: "), cdpData, cdpDataIndex, cdpFieldLength);
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

void set_mac(const byte a[], unsigned int offset, unsigned int length) {
  unsigned int n = 0;
  for(unsigned int i=offset; i<offset+length; ++i) {
    if(i>offset && i%2==0) value_mac_buffer[n++] = ':';
    
    value_mac_buffer[n++] = val2hex(a[i] >> 4);
    value_mac_buffer[n++] = val2hex(a[i] & 0xf);
  }
  
  value_mac_buffer[n++] = '\0';
  set_menu(LABEL_MAC, value_mac_buffer);
}

void handleCdpAsciiField(const __FlashStringHelper * title, const byte a[], unsigned int offset, unsigned int length, char* buffer, size_t buffer_size) {
  unsigned int i;
  for(i=0; i<length && i<buffer_size; ++i) {
    buffer[i] = a[offset + i];
  }
  buffer[i] = '\0';
  
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
  unsigned int n = 0;
  unsigned long numOfAddrs = (a[offset] << 24) | (a[offset+1] << 16) | (a[offset+2] << 8) | a[offset+3];
  offset += 4;
  for(unsigned long i=0; i<numOfAddrs; ++i) {
    //TODO: not overwrite, but add address
    n = 0;
    
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
      
      if(j>0) value_ip_buffer[n++] = '.';
      if(address[j] >= 100) value_ip_buffer[n++] = val2dec(address[j] / 100);
      if(address[j] >= 10) value_ip_buffer[n++] = val2dec((address[j] / 10) % 10);
      value_ip_buffer[n++] = val2dec(address[j] % 10);
    }
  
    Serial.print("- ");
    print_ip(address, 0, 4);
  }
  
  value_ip_buffer[n++] = '\0';
  set_menu(LABEL_ADDRESS, value_ip_buffer);
    
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
  unsigned int n = 0;
  for(unsigned int i=offset; i<offset+length; ++i) {
    if(i>offset) Serial.print(':');
    if(a[i] < 0x10) Serial.print('0');
    Serial.print(a[i], HEX);
  }
}

char val2dec(byte b) {
  switch(b) {
    case 0: return '0';
    case 1: return '1';
    case 2: return '2';
    case 3: return '3';
    case 4: return '4';
    case 5: return '5';
    case 6: return '6';
    case 7: return '7';
    case 8: return '8';
    case 9: return '9';
  }
}

char val2hex(byte b) {
  switch(b) {
    case 0x0: return '0';
    case 0x1: return '1';
    case 0x2: return '2';
    case 0x3: return '3';
    case 0x4: return '4';
    case 0x5: return '5';
    case 0x6: return '6';
    case 0x7: return '7';
    case 0x8: return '8';
    case 0x9: return '9';
    case 0xA: return 'A';
    case 0xB: return 'B';
    case 0xC: return 'C';
    case 0xD: return 'D';
    case 0xE: return 'E';
    case 0xF: return 'F';
  }
}
