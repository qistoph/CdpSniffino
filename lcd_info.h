#ifndef _LCD_INFO_H_
#define _LCD_INFO_H_

#include <Arduino.h>

typedef enum {
    LABEL_MAC = 1,
    LABEL_DEVICE_ID,
    LABEL_ADDRESS, // TODO: multiple addresses
    LABEL_PORT_ID,
    LABEL_SOFTWARE,
    LABEL_PLATFORM,
    LABEL_NATIVE_VLAN,
    LABEL_DUPLEX,
} label_type;

#define INVISIBLE 0
#define VISIBLE 1

struct menu_item_s {
    const label_type type;
    const char* label;
    const char* value;
    byte visible;
};

typedef struct menu_item_s menu_item;

extern menu_item menu[];

extern size_t menu_size;
extern unsigned int menu_current;
extern unsigned int lcd_more_offset;

extern volatile unsigned long last_cdp_received;
extern volatile unsigned long cdp_packets_received;

extern volatile unsigned int lcd_delta_t;
extern volatile unsigned int lcd_ttl;

void set_menu(label_type type, const char* value);
void lcd_info_next();
void lcd_info_more();

#endif
